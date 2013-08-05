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
/* Local type.                                                               */
/*===========================================================================*/

typedef struct {
  float joints[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} StepperKinematicsState;

typedef struct {
  uint32_t interval;
  uint32_t step_max;
  uint32_t step_size[RAD_NUMBER_STEPPERS];
  int32_t current[RAD_NUMBER_STEPPERS];
} StepperStepState;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

void enable_all(void)
{
  RadStepperChannel *ch;
  uint8_t i;

  pexSysLockFromIsr();
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
  pexSysUnlockFromIsr();
}

static uint8_t phase = 0;
static uint32_t last_era = 0;
static PlannerOutputBlock *curr_block = NULL;

static StepperKinematicsState last_velocity;
static StepperStepState step_state;
static uint32_t running_counter;

void stepper_event(GPTDriver *gptp)
{
  PlannerOutputBlock *new_block = NULL;

  if (phase == 0)
  {
    // Phase Setup
    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      palDisableSig(radboard.stepper.channels[i].step);
    }

    // If need to change - free current block and null
    if (curr_block == NULL || curr_block->velocity_mode)
    {
      chSysLockFromIsr();
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

        enable_all();
        if (curr_block->velocity_mode) {
          step_state.interval = radboard.stepper.gpt_config->frequency / 65536;
          step_state.step_max = 65536;
          for (uint8_t i = 0; i < radboard.stepper.count; i++) {
            step_state.current[i] = -step_state.step_max / 2;
          }
        }
      } else if (curr_block == NULL) {
        // No blocks? Sleep and wait
        gptStartOneShotI(radboard.stepper.gpt, radboard.stepper.gpt_config->frequency / 1024);
        chSysUnlockFromIsr();
        return;
      }
      chSysUnlockFromIsr();
    }

    pexSysLockFromIsr();
    if (curr_block->velocity_mode) {
      if (running_counter % (65536 / 32) == 0) {
        for (uint8_t i = 0; i < machine.kinematics.joint_count; i++) {
          float *lv = &last_velocity.joints[i];
          float *bv = &curr_block->data.velocity.joints[i];
          if (*lv == *bv) continue;
          *lv += curr_block->acceleration.joints[i] / 32;
          if (fabs(*lv) > fabs(*bv)) *lv = *bv;

          uint8_t ch_id = machine.kinematics.joints[i].stepper_id;
          step_state.step_size[ch_id] =
              *lv * fabs(machine.kinematics.joints[i].scale);

          if (machine.extruder.devices[i].scale < 0)
            palEnableSig(radboard.stepper.channels[ch_id].dir);
          else
            palDisableSig(radboard.stepper.channels[ch_id].dir);
        }

        for (uint8_t i = 0; i < machine.extruder.count; i++) {
          float *lv = &last_velocity.extruders[i];
          float *bv = &curr_block->data.velocity.extruders[i];
          if (*lv == *bv) continue;
          *lv += curr_block->acceleration.extruders[i] / 32;
          if (fabs(*lv) > fabs(*bv)) *lv = *bv;

          uint8_t ch_id = machine.extruder.devices[i].stepper_id;
          step_state.step_size[ch_id] =
              *lv * fabs(machine.extruder.devices[i].scale);

          if (machine.extruder.devices[i].scale < 0)
            palEnableSig(radboard.stepper.channels[ch_id].dir);
          else
            palDisableSig(radboard.stepper.channels[ch_id].dir);
        }
      }
      running_counter = (running_counter + 1) % step_state.step_max;
    }

    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      palDisableSig(radboard.stepper.channels[i].step);
    }
    pexSysUnlockFromIsr();
  } else
  {
    // Phase Execute
    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      int32_t *c = &step_state.current[i];
      *c += step_state.step_size[i];
      if (*c > 0 || (i % 2 == 1)) {
        *c -= step_state.step_max;
        palEnableSig(radboard.stepper.channels[i].step);
      }
    }
  }
  phase = 1 - phase;
  gptStartOneShotI(radboard.stepper.gpt, step_state.interval / 2);
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
  radboard.stepper.gpt_config->callback = stepper_event;
  gptStart(radboard.stepper.gpt, radboard.stepper.gpt_config);
  gptStartOneShot(radboard.stepper.gpt, radboard.stepper.gpt_config->frequency / 1024);
}

/** @} */
