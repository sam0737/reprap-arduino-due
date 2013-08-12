/*
    RAD - Copyright (C) 2013 Sam Wong

    This file is part of RAD project.

    RAD is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    RAD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file    stepper.c
 * @brief   Stepper
 *
 * @addtogroup STEPPER
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "output.h"

/*===========================================================================*/
/* Local definition.                                                         */
/*===========================================================================*/

#define STEPPER_VELOCITY_STEP_FREQ     32768
#define STEPPER_VELOCITY_PROFILE_FREQ  256

Semaphore stepper_sem;
/*===========================================================================*/
/* Local type.                                                               */
/*===========================================================================*/

typedef struct {
  float joints[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} StepperKinematicsState;

typedef struct {
  uint8_t dir:1;
  uint8_t limit_hit:1;
  int32_t step;
  int32_t current;
} StepperStepStateChannel;

typedef struct {
  int32_t interval;
  int32_t step_max;
  StepperStepStateChannel channels[RAD_NUMBER_STEPPERS];
} StepperStepState;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static WORKING_AREA(waStepper, 128);

static void stepper_enable_all(void)
{
  RadStepperChannel *ch;
  uint8_t i;

  pexSysLock();
  if (palHasSig(radboard.stepper.main_enable))
    palEnableSig(radboard.stepper.main_enable);

  for (i = 0; i < machine.kinematics.joint_count; i++) {
    ch = &radboard.stepper.channels[machine.kinematics.joints[i].stepper_id];
    if (palHasSig(ch->enable))
      palEnableSig(ch->enable);
  }
  for (i = 0; i < machine.extruder.count; i++) {
    ch = &radboard.stepper.channels[machine.extruder.devices[i].stepper_id];
    if (palHasSig(ch->enable))
      palEnableSig(ch->enable);
  }
  pexSysUnlock();
}

static void stepper_disable_all(void)
{
  RadStepperChannel *ch;
  uint8_t i;

  pexSysLock();
  if (palHasSig(radboard.stepper.main_enable))
    palDisableSig(radboard.stepper.main_enable);

  for (i = 0; i < machine.kinematics.joint_count; i++) {
    ch = &radboard.stepper.channels[machine.kinematics.joints[i].stepper_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
  }
  for (i = 0; i < machine.extruder.count; i++) {
    ch = &radboard.stepper.channels[machine.extruder.devices[i].stepper_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
  }
  pexSysUnlock();
}

static uint8_t phase = 0;
static bool_t estop = 0;
static int32_t last_era = 0;
static PlannerOutputBlock *curr_block = NULL;

static StepperKinematicsState last_velocity;
static StepperStepState step_state;
static uint32_t running_counter;

static void stepper_event(GPTDriver *gptp)
{
  chSysLockFromIsr();
  chSemSignalI(&stepper_sem);
  chSysUnlockFromIsr();
}

static void stepper_fetch_new_block(void)
{  chSysLock();
  PlannerOutputBlock *new_block = NULL;

  plannerPeekBlockI(new_block);
  while (new_block != NULL && new_block->era - last_era <= 0) {
    plannerFetchBlockI(new_block);
    plannerFreeBlockI(new_block);
    plannerPeekBlockI(new_block);
  }

  if (new_block != NULL) {
    if (curr_block != NULL) plannerFreeBlockI(curr_block);
    plannerFetchBlockI(curr_block);
    running_counter = 0;

    if (curr_block->mode == BLOCK_Velocity) {
      stepper_enable_all();
      step_state.interval = radboard.stepper.gpt_config->frequency / STEPPER_VELOCITY_STEP_FREQ;
      step_state.step_max = STEPPER_VELOCITY_STEP_FREQ;
      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        step_state.channels[i].current = -step_state.step_max / 2;
      }
      gptStartContinuousI(radboard.stepper.gpt, step_state.interval / 2);
    }
  } else if (curr_block == NULL) {
    // No blocks? Sleep and wait
    gptStopTimer(radboard.stepper.gpt);
    gptStartOneShotI(radboard.stepper.gpt, radboard.stepper.gpt_config->frequency / 1024);
  }
  chSysUnlock();
}

static void stepper_free_current_block(void)
{
  chSysLock();
  plannerFreeBlockI(curr_block);
  curr_block = NULL;
  chSysUnlock();
}

static void stepper_velocity_profile(void)
{
  for (uint8_t i = 0; i < machine.kinematics.joint_count; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    float *pv = &last_velocity.joints[i];
    float *sv = &curr_block->joints[i].data.velocity;

    uint8_t ch_id = j->stepper_id;
    RadStepperChannel *ch = &radboard.stepper.channels[ch_id];
    StepperStepStateChannel *ss = &step_state.channels[ch_id];

    if (!ss->limit_hit && (
        (curr_block->stop_on_limit_changes &&
            j->state.limit_state != j->state.old_limit_state) ||
            j->state.limit_state != LIMIT_Normal)) {
      *sv = 0;
      ss->limit_hit = 1;
      j->state.limit_state = j->state.old_limit_state;
      j->state.limit_step = ch->pos;
    }

    j->state.stopped = (*pv == 0 && *sv == 0);
    j->state.pos = ch->pos / j->scale;
    if (*pv == *sv) continue;

    if (*pv < *sv) {
      *pv += curr_block->joints[i].acceleration / STEPPER_VELOCITY_PROFILE_FREQ;
      if (*pv > *sv) *pv = *sv;
    } else {
      *pv -= curr_block->joints[i].acceleration / STEPPER_VELOCITY_PROFILE_FREQ;
      if (*pv < *sv) *pv = *sv;
    }

    /* *pv and scale could be negative */
    ss->step = *pv * j->scale;

    if (ss->step < 0) {
      ss->step *= -1;
      ss->dir = -1;
      palEnableSig(ch->dir);
    } else {
      ss->dir = 1;
      palDisableSig(ch->dir);
    }
  }

  for (uint8_t i = 0; i < machine.extruder.count; i++) {
    float *pv = &last_velocity.extruders[i];
    float *sv = &curr_block->extruders[i].data.velocity;

    if (*pv == *sv) continue;

    if (*pv < *sv) {
      *pv += curr_block->joints[i].acceleration / STEPPER_VELOCITY_PROFILE_FREQ;
      if (*pv > *sv) *pv = *sv;
    } else {
      *pv -= curr_block->joints[i].acceleration / STEPPER_VELOCITY_PROFILE_FREQ;
      if (*pv < *sv) *pv = *sv;
    }

    uint8_t ch_id = machine.extruder.devices[i].stepper_id;
    StepperStepStateChannel *ss = &step_state.channels[ch_id];

    /* *pv and scale could be negative */
    ss->step = *pv * machine.extruder.devices[i].scale;

    if (ss->step < 0) {
      ss->step *= -1;
      ss->dir = -1;
      palEnableSig(radboard.stepper.channels[ch_id].dir);
    } else {
      ss->dir = 1;
      palDisableSig(radboard.stepper.channels[ch_id].dir);
    }
  }
}

static msg_t threadStepper(void *arg) {
  (void)arg;
  chRegSetThreadName("stepper");

  while (1) {
    while (chSemWaitTimeout(&stepper_sem, TIME_INFINITE) != RDY_OK);

    if (phase == 0)
    {
      // Phase Setup
      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        palDisableSig(radboard.stepper.channels[i].step);
      }

      // If need to change - free current block and null
      if (curr_block == NULL || curr_block->mode == BLOCK_Velocity)
      {
        stepper_fetch_new_block();
        if (curr_block == NULL) continue;
      }

      if (curr_block->mode == BLOCK_Estop_Clear) {
        estop = 0;
        stepper_free_current_block();
        continue;
      }

      if (curr_block->mode == BLOCK_Estop || estop) {
        estop = 1;
        stepper_disable_all();
        stepper_free_current_block();
        continue;
      }

      pexSysLock();
      if (curr_block->mode == BLOCK_Velocity) {
        if (running_counter % (STEPPER_VELOCITY_STEP_FREQ / STEPPER_VELOCITY_PROFILE_FREQ) == 0) {
          stepper_velocity_profile();
        }
      }
      running_counter = (running_counter + 1) % step_state.step_max;

      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        palDisableSig(radboard.stepper.channels[i].step);
      }
      pexSysUnlock();
    } else
    {
      // Phase Execute
      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        int32_t *c = &step_state.channels[i].current;
        *c += step_state.channels[i].step;
        if (*c > 0) {
          *c -= step_state.step_max;
          palEnableSig(radboard.stepper.channels[i].step);
          radboard.stepper.channels[i].pos +=
              step_state.channels[i].dir;
        }
      }
    }
    phase = 1 - phase;
  }
  return 0;
}

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void stepperInit(void)
{
  uint8_t i;
  RadStepperChannel *ch;
  if (palHasSig(radboard.stepper.main_enable)) {
    pexDisableSig(radboard.stepper.main_enable);
    palSetSigMode(radboard.stepper.main_enable, PAL_MODE_OUTPUT_PUSHPULL);
  }

  for (i = 0; i < radboard.stepper.count; i++)
  {
    ch = &radboard.stepper.channels[i];
    if (palHasSig(ch->enable)) {
      pexDisableSig(ch->enable);
      palSetSigMode(ch->enable, PAL_MODE_OUTPUT_PUSHPULL);
    }
    pexDisableSig(ch->step);
    palSetSigMode(ch->step, PAL_MODE_OUTPUT_PUSHPULL);
    pexDisableSig(ch->dir);
    palSetSigMode(ch->dir, PAL_MODE_OUTPUT_PUSHPULL);
  }

  // Start stepper loop
  chSemInit(&stepper_sem, 0);
  chThdCreateStatic(waStepper, sizeof(waStepper), NORMALPRIO + 32, threadStepper, NULL);

  radboard.stepper.gpt_config->callback = stepper_event;
  gptStart(radboard.stepper.gpt, radboard.stepper.gpt_config);
  gptStartOneShot(radboard.stepper.gpt, radboard.stepper.gpt_config->frequency / 1024);
}

void stepperSetHome(uint8_t joint, int32_t home_step, float home_pos)
{
  RadJoint* j = &machine.kinematics.joints[joint];
  RadStepperChannel* ch = &radboard.stepper.channels[j->stepper_id];
  chSysLock();
  ch->pos = (ch->pos - home_step) + home_pos * j->scale;
  chSysUnlock();
}

/** @} */
