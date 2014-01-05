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

#if HAL_USE_GPT
/*===========================================================================*/
/* Local definition.                                                         */
/*===========================================================================*/

#define STEPPER_VELOCITY_STEP_FREQ     32768
#define STEPPER_VELOCITY_PROFILE_FREQ  256

/*===========================================================================*/
/* Local type.                                                               */
/*===========================================================================*/

typedef struct {
  float joints[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} StepperKinematicsState;

typedef struct {
  /* @brief Direction (-1 or 1) */
  int8_t dir;
  uint8_t limit_hit:1;
  /* @brief Output PWM: step value */
  int32_t step;
  /* @brief Output PWM: current value */
  int32_t current;
  /* @brief Absolute position of the channel */
  int32_t pos;
} StepperStepStateChannel;

typedef struct {
  int32_t interval;
  int32_t step_max;
  StepperStepStateChannel channels[RAD_NUMBER_STEPPERS];
} StepperStepState;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static WORKING_AREA(waStepper, 256);

static BinarySemaphore bsemStepperLoop;
static Mutex mtxState;

static uint8_t phase = 0;
static bool_t estop = 0;
static PlannerOutputBlock active_block;
static RadJointsState joints_state;
static StepperStepState step_state;

/* Velocity mode state variables */
static StepperKinematicsState last_velocity;
static uint32_t running_counter;

static void stepper_enable_all(void)
{
  RadStepperChannel *ch;
  uint8_t i;

  pexSysLock();
  if (palHasSig(radboard.stepper.main_enable))
    palEnableSig(radboard.stepper.main_enable);

  for (i = 0; i < RAD_NUMBER_JOINTS; i++) {
    ch = &radboard.stepper.channels[machine.kinematics.joints[i].stepper_id];
    if (palHasSig(ch->enable))
      palEnableSig(ch->enable);
  }
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
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

  for (i = 0; i < RAD_NUMBER_JOINTS; i++) {
    uint8_t ch_id = machine.kinematics.joints[i].stepper_id;
    last_velocity.joints[i] = 0;
    ch = &radboard.stepper.channels[ch_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
    step_state.channels[ch_id].step = 0;
  }
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    uint8_t ch_id = machine.extruder.devices[i].stepper_id;
    last_velocity.extruders[i] = 0;
    ch = &radboard.stepper.channels[ch_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
    step_state.channels[ch_id].step = 0;
  }
  pexSysUnlock();
}

static void stepper_event(GPTDriver *gptp)
{
  (void) gptp;
  chSysLockFromIsr();
  chBSemSignalI(&bsemStepperLoop);
  chSysUnlockFromIsr();
}

static void stepper_idle(void)
{
  chSysLock();
  gptStopTimerI(radboard.stepper.gpt);
  active_block.mode = BLOCK_Idle;
  chSysUnlock();
}

static void stepper_fetch_new_block(void)
{
  bool_t new_block = FALSE;
  while (1)
  {    chSysLock();
    new_block = plannerMainQueueFetchBlockI(&active_block);
    if (active_block.mode != BLOCK_Idle)
      break;
    chSysUnlock();
    chThdSleepMilliseconds(10);
  }
  chSysUnlock();
  if (!new_block)
    return;

  running_counter = 0;
  if (active_block.mode == BLOCK_Velocity) {
    stepper_enable_all();
    step_state.interval = radboard.stepper.gpt_config->frequency / STEPPER_VELOCITY_STEP_FREQ;
    step_state.step_max = STEPPER_VELOCITY_STEP_FREQ;
    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      ss->current = -step_state.step_max / 2;
      ss->limit_hit = 0;
    }
    gptStartContinuousI(radboard.stepper.gpt, step_state.interval / 2);
  }
}

static void stepper_velocity_profile(void)
{
  chMtxLock(&mtxState);
  bool_t all_stopped = TRUE;

  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    RadJointState *js = &joints_state.joints[i];
    float *pv = &last_velocity.joints[i];
    float *sv = &active_block.v.joints[i].sv;

    uint8_t ch_id = j->stepper_id;
    RadStepperChannel *ch = &radboard.stepper.channels[ch_id];
    StepperStepStateChannel *ss = &step_state.channels[ch_id];

    if (!ss->limit_hit && (
        (active_block.stop_on_limit_changes &&
            js->limit_state != js->old_limit_state) ||
        (!active_block.stop_on_limit_changes &&
            js->limit_state != LIMIT_Normal)
      )) {
      *sv = 0;
      ss->limit_hit = 1;
      js->old_limit_state = js->limit_state;
      js->limit_step = ss->pos;
    }

    if (!active_block.v.joints[i].is_stop_signalled) {
      if (*pv == 0 && *sv == 0) {
        js->stopped = TRUE;
        active_block.v.joints[i].is_stop_signalled = TRUE;
      } else {
        all_stopped = FALSE;
      }
    }

    if (*pv == *sv) continue;

    if (*pv < *sv) {
      *pv += active_block.v.joints[i].acc / STEPPER_VELOCITY_PROFILE_FREQ;
      if (*pv > *sv) *pv = *sv;
    } else {
      *pv -= active_block.v.joints[i].acc / STEPPER_VELOCITY_PROFILE_FREQ;
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

  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    float *pv = &last_velocity.extruders[i];
    float *sv = &active_block.v.extruders[i].sv;

    if (*sv != 0) all_stopped = FALSE;
    if (*pv == *sv) continue;

    all_stopped = FALSE;
    if (*pv < *sv) {
      *pv += active_block.v.joints[i].acc / STEPPER_VELOCITY_PROFILE_FREQ;
      if (*pv > *sv) *pv = *sv;
    } else {
      *pv -= active_block.v.joints[i].acc / STEPPER_VELOCITY_PROFILE_FREQ;
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

  if (all_stopped) {
    stepper_idle();
  }
  chMtxUnlock();
}

static msg_t threadStepper(void *arg) {
  (void)arg;
  chRegSetThreadName("stepper");

  /*
   * This thread must be run at the highest priority, and thus we worry less
   * about the variable synchronization.
   * They are passed will simple variable which normally is not a good idea.
   */
  while (1) {
    if (phase == 0)
    {
      // Phase Setup
      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        palDisableSig(radboard.stepper.channels[i].step);
      }

      do
      {
        stepper_fetch_new_block();

        if (active_block.mode == BLOCK_Estop_Clear) {
          estop = 0;
          stepper_idle();
          continue;
        }

        if (active_block.mode == BLOCK_Estop || estop) {
          estop = 1;
          stepper_disable_all();
          stepper_idle();
          continue;
        }

        pexSysLock();
        if (active_block.mode == BLOCK_Velocity) {
          if (running_counter % (STEPPER_VELOCITY_STEP_FREQ / STEPPER_VELOCITY_PROFILE_FREQ) == 0) {
            stepper_velocity_profile();
          }
        }
        running_counter = (running_counter + 1) % step_state.step_max;
        pexSysUnlock();
      } while (active_block.mode == BLOCK_Idle);
    } else
    {
      // Phase Execute
      chMtxLock(&mtxState);
      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        StepperStepStateChannel *ss = &step_state.channels[i];
        ss->current += ss->step;
        if (ss->current > 0) {
          ss->current -= step_state.step_max;
          palEnableSig(radboard.stepper.channels[i].step);
          ss->pos += ss->dir;
        }
      }
      chMtxUnlock();
    }
    phase = 1 - phase;
    chBSemWait(&bsemStepperLoop);
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
  chBSemInit(&bsemStepperLoop, 0);
  chMtxInit(&mtxState);
  chThdCreateStatic(waStepper, sizeof(waStepper), NORMALPRIO + 32, threadStepper, NULL);

  radboard.stepper.gpt_config->callback = stepper_event;
  gptStart(radboard.stepper.gpt, radboard.stepper.gpt_config);
}

void stepperSetHome(uint8_t joint_id, int32_t home_step, float home_pos)
{
  RadJoint* j = &machine.kinematics.joints[joint_id];
  uint8_t ch_id = j->stepper_id;
  StepperStepStateChannel *ss = &step_state.channels[ch_id];
  ss->pos = (ss->pos - home_step) + home_pos * j->scale;
}

RadJointsState stepperGetJointsState()
{
  chMtxLock(&mtxState);
  for (uint8_t i = 0; i <RAD_NUMBER_JOINTS; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    RadJointState *js = &joints_state.joints[i];
    uint8_t ch_id = j->stepper_id;
    StepperStepStateChannel *ss = &step_state.channels[ch_id];
    js->pos = ss->pos / j->scale;
  }
  RadJointsState s = joints_state;
  chMtxUnlock();
  return s;
}

void stepperSetLimitState(uint8_t joint_id, RadLimitState state)
{
  chMtxLock(&mtxState);
  joints_state.joints[joint_id].limit_state = state;
  chMtxUnlock();
}

void stepperResetOldLimitState(uint8_t joint_id)
{
  chMtxLock(&mtxState);
  joints_state.joints[joint_id].old_limit_state = LIMIT_Normal;
  chMtxUnlock();
}

void stepperClearStopped(uint8_t joint_id)
{
  chMtxLock(&mtxState);
  joints_state.joints[joint_id].stopped = FALSE;
  chMtxUnlock();
}

void stepperSetHomed(uint8_t joint_id)
{
  chMtxLock(&mtxState);
  joints_state.joints[joint_id].homed = TRUE;
  chMtxUnlock();
}
#else

void stepperInit(void){}
void stepperSetHome(uint8_t joint_id, int32_t home_step, float home_pos){}
RadJointsState stepperGetJointsState(void)
{
  RadJointsState s ; return s;
}
void stepperSetLimitState(uint8_t joint_id, RadLimitState limit){}
void stepperResetOldLimitState(uint8_t joint_id){}
void stepperClearStopped(uint8_t joint_id){}
void stepperSetHomed(uint8_t joint_id){}
#endif

/** @} */
