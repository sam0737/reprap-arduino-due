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

#ifdef RAD_DEBUG_WAITLINE
#undef RAD_DEBUG_WAITLINE
#define RAD_DEBUG_WAITLINE(...)
#endif

#define DEBUG_STEP 0

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
      uint32_t last_tick_pace;
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

systime_t past_timer;
static void stepper_wait_timer(void)
{
  while (!timer_active) {
    RAD_DEBUG_PRINTF("STEPPER: Sleep while not active?\n");
    chThdSleepMilliseconds(1000);
  }
  past_timer += timer_interval;
  #if RAD_TEST && DEBUG_STEP
  if (phase == 0 && active_block.mode == BLOCK_Positional)
  {
    RAD_DEBUG_PRINTF("STEPPER: %4d/%-4d %8d|",
        step_state.total_step_spent, step_state.step_max,
        timer_interval);
    chSysLock();
    for (uint8_t i = 0; i < RAD_NUMBER_STEPPERS; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      RAD_DEBUG_PRINTF(" %6d", ss->pos);
    }
    chSysUnlock();
    RAD_DEBUG_PRINTF("|%.3f A %.3f\n",
        (double) step_state.unit_tick_pace / step_state.last_tick_pace,
        (double) active_block.p.distance / step_state.step_max / timer_interval * clock.tick_frequency
        );
  }
  #endif
  while (past_timer >= 25000)
  {
    past_timer -= 25000;
    chThdSleepMicroseconds(25000);
  }
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

static void stepper_reset_velocity(void)
{
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    uint8_t ch_id = machine.kinematics.joints[i].stepper_id;
    RadStepperChannel *ch = &radboard.stepper.channels[ch_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
    step_state.last_velocity.joints[i] = 0;
    step_state.channels[ch_id].step = 0;
  }
  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    uint8_t ch_id = machine.extruder.devices[i].stepper_id;
    RadStepperChannel *ch = &radboard.stepper.channels[ch_id];
    if (palHasSig(ch->enable))
      palDisableSig(ch->enable);
    step_state.last_velocity.extruders[i] = 0;
    step_state.channels[ch_id].step = 0;
  }
}

static void stepper_disable_all(void)
{
  pexSysLock();
  if (palHasSig(radboard.stepper.main_enable))
    palDisableSig(radboard.stepper.main_enable);
  stepper_reset_velocity();
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
  bool_t prev_is_velocity = active_block.mode == BLOCK_Velocity;
  PlannerOutputBlockSectionV prev_block_v;
  if (prev_is_velocity)
    prev_block_v = active_block.v;
  while (1)
  {    chSysLock();
    new_block = plannerMainQueueFetchBlockI(&active_block, active_block.mode);
    if (active_block.mode != BLOCK_Idle)
      break;
    chSysUnlock();
    stepper_stop_timer();
    chThdSleepMilliseconds(10);
  }
  if (!new_block)
    return;

  step_state.total_step_spent = 0;
  if (active_block.mode == BLOCK_Velocity) {
    if (!prev_is_velocity)
    {
      // The control data is shared with other block mode
      // Reset all the fields to stop if this wasn't velocity mode and
      //   hence were stopped previously
      stepper_reset_velocity();
    }
    stepper_enable_all();
    step_state.step_max = STEPPER_VELOCITY_STEP_FREQ;
    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - velocity");
    for (uint8_t i = 0; i < RAD_NUMBER_STEPPERS; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      ss->current = -((int32_t)step_state.step_max) / 2;
      ss->limit_hit = 0;
    }
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
      if (isnan(active_block.v.joints[i].sv)) {
        active_block.v.joints[i].sv = prev_is_velocity ? prev_block_v.joints[i].sv : 0;
      } else {
        // Note: look ma, no chSysLock(), we probably don't need it, hopefully
        joints_state.joints[i].stopped = FALSE;
      }
      RAD_DEBUG_PRINTF(" #%d=%f", i, active_block.v.joints[i].sv);
    }
    for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
      if (isnan(active_block.v.extruders[i].sv)) {
        active_block.v.extruders[i].sv = prev_is_velocity ? prev_block_v.extruders[i].sv : 0;
      } else {
        // Note: look ma, no chSysLock(), we probably don't need it, hopefully
        joints_state.joints[i].stopped = FALSE;
      }
      RAD_DEBUG_PRINTF(" E#%d=%f", i, active_block.v.extruders[i].sv);
    }
    RAD_DEBUG_PRINTF("\n");
    stepper_set_timer(clock.tick_frequency / 2 / STEPPER_VELOCITY_STEP_FREQ);
  } else if (active_block.mode == BLOCK_Positional) {
    stepper_enable_all();
    step_state.step_max = 1;
    pexSysLock();

    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - delta");
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
      RadJoint *j = &machine.kinematics.joints[i];
      StepperStepStateChannel *ss = &step_state.channels[j->stepper_id];
      int32_t delta = active_block.p.target.joints[i] * j->scale - ss->pos;
      RAD_DEBUG_PRINTF(" #%d=%d(%f)", j->stepper_id, delta, active_block.p.target.joints[i]);
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
      RAD_DEBUG_PRINTF(" #%d=%d(%f)", j->stepper_id, delta, active_block.p.target.extruders[i]);
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

    RAD_DEBUG_PRINTF("\nSTEPPER: NEW BLOCK - step_max: %d, distance: %.3fmm, d-after: %.3fmm, duration: %.5fs\n",
        step_state.step_max, active_block.p.distance, active_block.p.decelerate_after, active_block.p.duration);

    step_state.acc_per_tick =
        2 * active_block.p.acc * active_block.p.distance / step_state.step_max;
    step_state.unit_tick_pace =
        active_block.p.distance * clock.tick_frequency / step_state.step_max;
    step_state.decelerate_after_step = active_block.p.decelerate_after / active_block.p.distance * step_state.step_max + 0.5;
    for (uint8_t i = 0; i < RAD_NUMBER_STEPPERS; i++) {
      StepperStepStateChannel *ss = &step_state.channels[i];
      ss->current = -((int32_t)step_state.step_max) / 2;
    }

    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - speed: %.3f,%.3f,%.3f(%.3f). acc %.3f, d-after %d\n",
        step_state.last_block_speed, active_block.p.nominal_speed, active_block.p.exit_speed, active_block.p.max_exit_speed,
        active_block.p.acc, step_state.decelerate_after_step);

    step_state.tick_speed_sq = step_state.last_block_speed * step_state.last_block_speed;

    step_state.last_tick_pace =
        (step_state.unit_tick_pace > clock.minimum_tick_pace * step_state.last_block_speed) ?
            clock.minimum_tick_pace :
            step_state.unit_tick_pace / step_state.last_block_speed;
    if (step_state.last_tick_pace == 0)
      step_state.last_tick_pace = 1;

    step_state.exit_tick_pace =
        (step_state.unit_tick_pace > clock.minimum_tick_pace * active_block.p.exit_speed) ?
            clock.minimum_tick_pace :
            step_state.unit_tick_pace / active_block.p.exit_speed + 1;

    step_state.nominal_tick_pace =
      (step_state.unit_tick_pace > clock.minimum_tick_pace * active_block.p.nominal_speed) ?
          clock.minimum_tick_pace :
          step_state.unit_tick_pace / active_block.p.nominal_speed;

    RAD_DEBUG_PRINTF("STEPPER: NEW BLOCK - pace: unit %d, min %d. last %d, nom %d, exit %d\n",
        step_state.unit_tick_pace, clock.minimum_tick_pace,
        step_state.last_tick_pace, step_state.nominal_tick_pace, step_state.exit_tick_pace);
  }
}

static void reset_step_state(void)
{
  chSysLock();
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
  chSysUnlock();
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
  if (step_state.last_tick_pace == 0)
    step_state.last_tick_pace = 1;
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
            js->changed_limit_state != LIMIT_Normal) ||
        (!active_block.stop_on_limit_changes &&
            js->limit_state != LIMIT_Normal)
      )) {
      RAD_DEBUG_PRINTF("STEPPER: %d limit hit: changed %d, now %d\n", ch_id, js->changed_limit_state, js->limit_state);
      *sv = 0;
      ss->limit_hit = 1;
      stepperResetOldLimitState(i);
      js->limit_step = ss->pos;
    }

    if (!js->stopped) {
      if (*pv == 0 && *sv == 0) {
        js->stopped = TRUE;
        RAD_DEBUG_PRINTF("STEPPER: Joint %d stopped\n", i);
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
  chSysUnlock();
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
      for (uint8_t i = 0; i < RAD_NUMBER_STEPPERS; i++) {
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
        step_state.total_step_spent = (step_state.total_step_spent + 1) % step_state.step_max;
        pexSysUnlock();
      } while (active_block.mode == BLOCK_Idle);
    } else
    {
      // Phase Execute
      chSysLock();
      pexSysLock();
      for (uint8_t i = 0; i < RAD_NUMBER_STEPPERS; i++) {
        StepperStepStateChannel *ss = &step_state.channels[i];
        ss->current += ss->step;
        if (ss->current > 0) {
          ss->current -= step_state.step_max;
          palEnableSig(radboard.stepper.channels[i].step);
          ss->pos += ss->dir;
        }
      }
      pexSysUnlock();
      chSysUnlock();

    }
    phase = 1 - phase;
    stepper_wait_timer();

    // Is positional finished?
    if (active_block.mode == BLOCK_Positional &&
        step_state.total_step_spent == 0 && phase == 0) {
      step_state.last_block_speed = step_state.unit_tick_pace / step_state.last_tick_pace;
      active_block.mode = BLOCK_Idle;
      RAD_DEBUG_PRINTF("STEPPER: Next block? unit %d, last %d\n",
          step_state.unit_tick_pace, step_state.last_tick_pace);
      RAD_DEBUG_WAITLINE();
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

  for (i = 0; i < RAD_NUMBER_STEPPERS; i++)
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
  chThdCreateStatic(waStepper, sizeof(waStepper), NORMALPRIO + 32, threadStepper, NULL);
}

void stepperSetHome(uint8_t joint_id, int32_t home_step, float home_pos)
{
  RadJoint* j = &machine.kinematics.joints[joint_id];
  uint8_t ch_id = j->stepper_id;
  StepperStepStateChannel *ss = &step_state.channels[ch_id];
  ss->pos = (ss->pos - home_step) + home_pos * j->scale;
  RAD_DEBUG_PRINTF("STEPPER: Set Home: Joint %d Step %d Pos %f, new pos = %d\n", joint_id, home_step, home_pos, ss->pos);
}

RadJointsState stepperGetJointsState()
{
  chSysLock();
  for (uint8_t i = 0; i <RAD_NUMBER_JOINTS; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    RadJointState *js = &joints_state.joints[i];
    uint8_t ch_id = j->stepper_id;
    StepperStepStateChannel *ss = &step_state.channels[ch_id];
    js->pos = ss->pos / j->scale;
  }
  RadJointsState s = joints_state;
  chSysUnlock();
  return s;
}

void stepperSetLimitState(uint8_t joint_id, RadLimitState state)
{
  chSysLock();
  RadJointState* js = &joints_state.joints[joint_id];
  js->limit_state = state;
  js->changed_limit_state |= js->base_limit_state ^ state;
  chSysUnlock();
}

void stepperResetOldLimitState(uint8_t joint_id)
{
  chSysLock();
  RadJointState* js = &joints_state.joints[joint_id];
  js->base_limit_state = js->limit_state;
  js->changed_limit_state = LIMIT_Normal;
  chSysUnlock();
}

void stepperSetHomed(uint8_t joint_id)
{
  chSysLock();
  joints_state.joints[joint_id].homed = TRUE;
  chSysUnlock();
}

PlannerVirtualPosition stepperGetCurrentPosition(void)
{
  PlannerVirtualPosition virtual_pos;
  PlannerPhysicalPosition physical_pos;

  chSysLock();
  for (uint8_t i = 0; i <RAD_NUMBER_JOINTS; i++) {
    RadJoint *j = &machine.kinematics.joints[i];
    uint8_t ch_id = j->stepper_id;
    StepperStepStateChannel *ss = &step_state.channels[ch_id];
    physical_pos.joints[i] = ss->pos / j->scale;
  }
  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    RadExtruder *ex = &machine.extruder.devices[i];
    uint8_t ch_id = ex->stepper_id;
    StepperStepStateChannel *ss = &step_state.channels[ch_id];
    physical_pos.extruders[i] = ss->pos / ex->scale;
  }
  chSysUnlock();

  machine.kinematics.forward_kinematics(&physical_pos, &virtual_pos);
  return virtual_pos;
}

/** @} */
