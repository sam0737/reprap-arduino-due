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

/*===========================================================================*/
/* Local type.                                                               */
/*===========================================================================*/

typedef struct {
  float joints[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} StepperKinematicsState;

typedef struct {
  /** @brief Direction (-1 or 1) */
  int8_t dir;
  uint8_t limit_hit:1;
  /** @brief Output PWM: step value */
  int32_t step;
  /** @brief Output PWM: current value */
  int32_t current;
  /** @brief Current absolute position of the channel */
  int32_t pos;
} StepperStepStateChannel;

typedef struct {
  union {
    struct {
      float last_tick_pace;
      float last_block_speed;
      float tick_speed_sq;

      uint32_t decelerate_after_step;
      uint32_t unit_tick_pace;
      float acc_per_tick;
      uint32_t nominal_tick_pace;
      uint32_t exit_tick_pace;
    };
    struct {
      StepperKinematicsState last_velocity;
    };
  };
  uint32_t total_step_spent;
  uint32_t step_max;
  StepperStepStateChannel channels[RAD_NUMBER_STEPPERS];
} StepperStepState;

typedef struct {
  uint32_t tick_frequency;
  /** @brief The slowest pace */
  uint32_t minimum_tick_pace;
} StepperClockFrequency;

/*===========================================================================*/
/* Local Definitions.                                                        */
/*===========================================================================*/

static WORKING_AREA(waStepper, 256);

static Mutex mtxState;

static uint8_t phase = 0;
static bool_t estop = 0;
static PlannerOutputBlock active_block;
static RadJointsState joints_state;
static StepperStepState step_state;
static StepperClockFrequency clock;

/*===========================================================================*/
/* Timer functions and emulations.                                           */
/*===========================================================================*/
#if HAL_USE_GPT
static BinarySemaphore bsemStepperLoop;
static void stepper_event(GPTDriver *gptp)
{
  (void) gptp;
  chSysLockFromIsr();
  chBSemSignalI(&bsemStepperLoop);
  chSysUnlockFromIsr();
}

static void stepper_init_timer(void)
{
  clock.tick_frequency = radboard.stepper.gpt_config->frequency;
  clock.minimum_tick_pace = clock.tick_frequency / 64;

  radboard.stepper.gpt_config->callback = stepper_event;
  gptStart(radboard.stepper.gpt, radboard.stepper.gpt_config);

  chBSemInit(&bsemStepperLoop, 0);
}

static void stepper_stop_timer(void)
{
  gptStopTimerI(radboard.stepper.gpt);
}

static void stepper_set_timer(gptcnt_t interval)
{
  gptStartContinuousI(radboard.stepper.gpt, interval);
}

static void stepper_wait_timer(void)
{
  chBSemWait(&bsemStepperLoop);
}
#else
BOOL timer_active;
int32_t timer_interval;
static void stepper_init_timer(void)
{
  clock.tick_frequency = 1000000;
  clock.minimum_tick_pace = clock.tick_frequency / 64;
  timer_active = FALSE;
}
static void stepper_stop_timer(void)
{
  if (!timer_active) return;
  timer_active = FALSE;
  #if RAD_TEST
  RAD_DEBUG_PRINTF("STEPPER: STOP\n");
  #endif
}
static void stepper_set_timer(int32_t interval)
{
  timer_active = TRUE;
  timer_interval = interval;
}
static void stepper_wait_timer(void)
{
  while (!timer_active) {
    RAD_DEBUG_PRINTF("STEPPER: Sleep while not active?\n");
    chThdSleepMilliseconds(1000);
  }
  #if RAD_TEST
  if (phase == 0)
  {
    RAD_DEBUG_PRINTF("STEPPER: %4d/%-4d:%d %8d|",
        step_state.total_step_spent, step_state.step_max, active_block.mode,
        timer_interval);
    chMtxLock(&mtxState);
    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      RAD_DEBUG_PRINTF(" %6d", ss->pos);
    }
    chMtxUnlock();
    RAD_DEBUG_PRINTF("|%.3f A %.3f\n",
        step_state.unit_tick_pace / step_state.last_tick_pace,
        step_state.step_max > 0 ?
            active_block.p.distance / step_state.step_max / timer_interval * clock.tick_frequency :
            0
        );
  }
  #endif
  chThdSleepMilliseconds(1);
}
#endif

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/
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
    ch = &radboard.stepper.channels[ch_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
    step_state.last_velocity.joints[i] = 0;
    step_state.channels[ch_id].step = 0;
  }
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    uint8_t ch_id = machine.extruder.devices[i].stepper_id;
    ch = &radboard.stepper.channels[ch_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
    step_state.last_velocity.extruders[i] = 0;
    step_state.channels[ch_id].step = 0;
  }
  pexSysUnlock();
}

static void stepper_idle(void)
{
  chSysLock();
  stepper_stop_timer();
  active_block.mode = BLOCK_Idle;
  step_state.last_block_speed = 0;
  chSysUnlock();
}

/* Positional Acceleration Calculation */
/*
 * Timer is configured by assigning the ticks to wait for the next wake-up
 * Timer tick frequency:                      tick_freq
 *    (== radboard.stepper.gpt_config->frequency)
 *
 * The exit speed of the last block (mm/s):   last_block_speed
 * Block information:
 *   Distance of travel (mm):                 distance
 *   Speed of travel (mm/s):                  speed
 *   Acceleration rate (mm/s/s):              acc
 *   Maximum steps among all axis:            step_max
 *   Ticks since acceleration:                tick_spent
 *
 * Ticks for the next step at specific speed
 *   = distance * tick_freq / step_max / speed
 *
 * Speed delta during acceleration/deceleration:
 *   = acc /tick_freq * tick_spent
 *
 * Ticks for the next step during acceleration
 *   = distance * tick_freq / step_max / (acc / tick_freq * tick_spent + last_block_speed)
 *
 * Variable that are constants throughout a block:
 *   tick_speed = distance * tick_freq / step_max
 *   acc_per_tick = acc / tick_freq
 *
 */

static void stepper_fetch_new_block(void)
{
  bool_t new_block = FALSE;
  while (1)
  {    chSysLock();
    new_block = plannerMainQueueFetchBlockI(&active_block, active_block.mode);
    if (active_block.mode != BLOCK_Idle)
      break;
    chSysUnlock();
    stepper_stop_timer();
    chThdSleepMilliseconds(10);
  }
  chSysUnlock();
  if (!new_block)
    return;

  step_state.total_step_spent = 0;
  if (active_block.mode == BLOCK_Velocity) {
    stepper_enable_all();
    step_state.step_max = STEPPER_VELOCITY_STEP_FREQ;
    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      ss->current = - ((int32_t)step_state.step_max) / 2;
      ss->limit_hit = 0;
    }
    stepper_set_timer(clock.tick_frequency / 2 / STEPPER_VELOCITY_STEP_FREQ);
  } else if (active_block.mode == BLOCK_Positional) {
    stepper_enable_all();
    step_state.step_max = 0;
    pexSysLock();

    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - delta");
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
      RadJoint *j = &machine.kinematics.joints[i];
      StepperStepStateChannel *ss = &step_state.channels[j->stepper_id];
      int32_t delta = active_block.p.target.joints[i] * j->scale - ss->pos;
      RAD_DEBUG_PRINTF(" #%d=%d", j->stepper_id, delta);
      ss->dir = delta > 0 ? 1 : -1;
      ss->step = fabs(delta);
      if ((uint32_t)ss->step > step_state.step_max) step_state.step_max = (uint32_t)ss->step;
      if (ss->dir < 0) {
        palEnableSig(radboard.stepper.channels[j->stepper_id].dir);
      } else {
        palDisableSig(radboard.stepper.channels[j->stepper_id].dir);
      }
    }
    for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
      RadExtruder *j = &machine.extruder.devices[i];
      StepperStepStateChannel *ss = &step_state.channels[j->stepper_id];
      int32_t delta = active_block.p.target.extruders[i] * j->scale - ss->pos;
      RAD_DEBUG_PRINTF(" #%d=%d", j->stepper_id, delta);
      ss->dir = delta > 0 ? 1 : -1;
      ss->step = fabs(delta);
      if ((uint32_t)ss->step > step_state.step_max) step_state.step_max = (uint32_t)ss->step;
      if (ss->dir < 0) {
        palEnableSig(radboard.stepper.channels[j->stepper_id].dir);
      } else {
        palDisableSig(radboard.stepper.channels[j->stepper_id].dir);
      }
    }
    pexSysUnlock();

    RAD_DEBUG_PRINTF("\nSTEPPER: NEW BLOCK - step_max: %d, distance: %.3fmm, duration: %.5fs\n",
        step_state.step_max, active_block.p.distance, active_block.p.duration);
    /*if (step_state.step_max == 0) {
      active_block.mode = BLOCK_Idle;
      return;
    }*/

    step_state.acc_per_tick =
        step_state.step_max ?
        2 * active_block.p.acc * active_block.p.distance / step_state.step_max :
        0;
    step_state.unit_tick_pace =
        step_state.step_max ?
        active_block.p.distance * clock.tick_frequency / step_state.step_max :
        clock.tick_frequency * clock.tick_frequency;
    step_state.decelerate_after_step = active_block.p.decelerate_after / active_block.p.distance * step_state.step_max + 0.5;
    for (uint8_t i = 0; i < radboard.stepper.count; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      ss->current = - ((int32_t)step_state.step_max) / 2;
    }

    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - speed: %.3f,%.3f,%.3f(%.3f). acc %.3f, d-after %d\n",
        step_state.last_block_speed, active_block.p.nominal_speed, active_block.p.exit_speed, active_block.p.max_exit_speed,
        active_block.p.acc, step_state.decelerate_after_step);

    step_state.tick_speed_sq = step_state.last_block_speed * step_state.last_block_speed;

    step_state.last_tick_pace =
        (step_state.unit_tick_pace > clock.minimum_tick_pace * step_state.last_block_speed) ?
            clock.minimum_tick_pace :
            step_state.unit_tick_pace / step_state.last_block_speed;

    step_state.exit_tick_pace =
        (step_state.unit_tick_pace > clock.minimum_tick_pace * active_block.p.exit_speed) ?
            clock.minimum_tick_pace :
            step_state.unit_tick_pace / active_block.p.exit_speed + 1;

    if (step_state.decelerate_after_step >= step_state.step_max)
    {
      step_state.nominal_tick_pace = step_state.exit_tick_pace;
    } else {
      step_state.nominal_tick_pace =
          (step_state.unit_tick_pace > clock.minimum_tick_pace * active_block.p.nominal_speed) ?
              clock.minimum_tick_pace :
              step_state.unit_tick_pace / active_block.p.nominal_speed;
    }

    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - pace: unit %d, min %d. %d, %d, %d\n",
        step_state.unit_tick_pace, clock.minimum_tick_pace,
        step_state.last_tick_pace, step_state.nominal_tick_pace, step_state.exit_tick_pace);
  }
}

static void reset_step_state(void)
{
  chMtxLock(&mtxState);
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    StepperStepStateChannel *ss = &step_state.channels[j->stepper_id];
    ss->pos = active_block.p.target.joints[i] * j->scale;
  }
  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    RadExtruder *j = &machine.extruder.devices[i];
    StepperStepStateChannel *ss = &step_state.channels[j->stepper_id];
    ss->pos = active_block.p.target.extruders[i] * j->scale;
  }
  chMtxUnlock();
}

static void positional_calculation(void)
{
  if (step_state.step_max != 0)
  {
    // TODO: Endstops?
    if (step_state.total_step_spent >= step_state.decelerate_after_step)
    {
      // Deceleration until exit_tick_pace is met
      if (step_state.last_tick_pace != step_state.exit_tick_pace)
      {
        if (step_state.total_step_spent == step_state.decelerate_after_step)
        {
          step_state.tick_speed_sq = step_state.unit_tick_pace / step_state.last_tick_pace;
          step_state.tick_speed_sq *= step_state.tick_speed_sq;
        }
        step_state.tick_speed_sq -= step_state.acc_per_tick;
        if (step_state.tick_speed_sq < 1) {
          step_state.last_tick_pace = step_state.exit_tick_pace;
        } else {
          step_state.last_tick_pace = step_state.unit_tick_pace * fast_inverse_square(step_state.tick_speed_sq);
          if (step_state.last_tick_pace > step_state.exit_tick_pace)
            step_state.last_tick_pace = step_state.exit_tick_pace;
        }
      }
    } else if (step_state.last_tick_pace > step_state.nominal_tick_pace)
    {
      // Acceleration until nominal_tick_pace is met
      step_state.tick_speed_sq += step_state.acc_per_tick;
      step_state.last_tick_pace = step_state.unit_tick_pace * fast_inverse_square(step_state.tick_speed_sq);
      if (step_state.last_tick_pace < step_state.nominal_tick_pace)
        step_state.last_tick_pace = step_state.nominal_tick_pace;
    } else if (step_state.last_tick_pace < step_state.nominal_tick_pace)
    {
      // Deceleration until nominal_tick_pace is met (Previous block doesn't decelerate enough?)
      step_state.tick_speed_sq -= step_state.acc_per_tick;
      if (step_state.tick_speed_sq < 1) {
        step_state.last_tick_pace = step_state.nominal_tick_pace;
      } else {
        step_state.last_tick_pace = step_state.unit_tick_pace * fast_inverse_square(step_state.tick_speed_sq);
        if (step_state.last_tick_pace > step_state.nominal_tick_pace)
          step_state.last_tick_pace = step_state.nominal_tick_pace;
      }
    }
  }
  stepper_set_timer(step_state.last_tick_pace);
}

static void stepper_velocity_profile(void)
{
  bool_t all_stopped = TRUE;

  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    RadJointState *js = &joints_state.joints[i];
    float *pv = &step_state.last_velocity.joints[i];
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
    float *pv = &step_state.last_velocity.extruders[i];
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
        if (active_block.mode == BLOCK_Reset) {
          reset_step_state();
          active_block.mode = BLOCK_Idle;
          continue;
        }
        if (active_block.mode == BLOCK_Estop || estop) {
          estop = 1;
          stepper_disable_all();
          stepper_idle();
          continue;
        }

        pexSysLock();
        if (active_block.mode == BLOCK_Positional) {
          positional_calculation();
        } else if (active_block.mode == BLOCK_Velocity) {
          if (step_state.total_step_spent % (STEPPER_VELOCITY_STEP_FREQ / STEPPER_VELOCITY_PROFILE_FREQ) == 0) {
            stepper_velocity_profile();
          }
        }
        step_state.total_step_spent = step_state.step_max > 0 ?
            (step_state.total_step_spent + 1) % step_state.step_max :
            0;
        pexSysUnlock();
      } while (active_block.mode == BLOCK_Idle);
    } else
    {
      // Phase Execute
      chMtxLock(&mtxState);
      pexSysLock();
      for (uint8_t i = 0; i < radboard.stepper.count; i++) {
        StepperStepStateChannel *ss = &step_state.channels[i];
        ss->current += ss->step;
        if (ss->current > 0) {
          ss->current -= step_state.step_max;
          palEnableSig(radboard.stepper.channels[i].step);
          ss->pos += ss->dir;
        }
      }
      pexSysUnlock();
      chMtxUnlock();

    }
    phase = 1 - phase;
    stepper_wait_timer();

    // Is positional finished?
    if (active_block.mode == BLOCK_Positional &&
        step_state.total_step_spent == 0 && phase == 0) {
      RAD_DEBUG_PRINTF("STEPPER: Next block?");
      RAD_DEBUG_WAITLINE();
      step_state.last_block_speed = step_state.unit_tick_pace / step_state.last_tick_pace;
      active_block.mode = BLOCK_Idle;
    }
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

  stepper_init_timer();

  // Start stepper loop
  chMtxInit(&mtxState);
  chThdCreateStatic(waStepper, sizeof(waStepper), NORMALPRIO + 32, threadStepper, NULL);
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

/** @} */
