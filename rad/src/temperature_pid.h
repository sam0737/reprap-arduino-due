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
 * @file    temperature_pid.h
 * @brief   Temperature PID
 *
 * @addtogroup TEMPERATURE
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "temperature.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

void temperatureAutoTune(uint8_t temp_id, float target, uint8_t total_cycles)
{
  if (temp_id >= RAD_NUMBER_TEMPERATURES)
    return;

  if (printerIsEstopped())
  {
    printerEstop(L_TEMPERATURE_AUTOTUNE_IN_ESTOP);
    return;
  }

  RadTemp* cht = &machine.temperature.devices[temp_id];
  if (cht->heating_pwm_id < 0)
    return;

  float pv = 0.0;
  float pvHigh = 0, pvLow = 10000;
  systime_t tHigh = 0, tLow = 0;
  bool_t heating = FALSE;

  RadTempPidState* state = &pid_states[temp_id];
  RadTempPidState backup_state = *state;
  chSysLock();
  state->tuning_cycle_remains = total_cycles;
  chSysUnlock();
  total_cycles = 0;

  int32_t bias, d;
  float Ku, Tu;

  systime_t now = chTimeNow();
  systime_t t1 = now - S2ST(10);
  systime_t t2 = now;

  // TODO: Max output limit
  int32_t limit = 255;
  bias = d = limit / 2;

  outputSet(cht->heating_pwm_id, 0);
  while (1)
  {
    pv = temperatureGet(temp_id).pv;
    pvHigh = fmax(pvHigh, pv);
    pvLow = fmin(pvLow, pv);

    if (heating && pv > target)
    {
      if (chTimeNow() - t2 > S2ST(5))
      {
        RAD_DEBUG_PRINTF("Autotune: cooling: bias=%d, d=%d\n", bias, d);
        heating = FALSE;
        outputSet(cht->heating_pwm_id, (bias - d) / 2);
        t1 = chTimeNow();
        tHigh = t1 - t2;
        pvHigh = pv;
      }
    }
    if (!heating && pv < target)
    {
      if (chTimeNow() - t1 > S2ST(5))
      {
        heating = TRUE;
        t2 = chTimeNow();
        tLow = t2 - t1;
        if (total_cycles > 0)
        {
          bias += (d*(tHigh - tLow))/(tLow + tHigh);
          if (bias < 20) {
            bias = 20;
          }
          else if (bias > limit - 20) {
            bias = limit - 20;
          }
          d = (bias > limit / 2) ? limit - 1 - bias : bias;

          if (total_cycles > 2)
          {
            Ku = (4.0 * d) / (3.14159 * (pvHigh-pvLow)/2.0);
            Tu = ((float)(tLow + tHigh) / CH_FREQUENCY);
            chSysLock();
            state->Kp = 0.6*Ku;
            state->Ki = 2*state->Kp/Tu;
            state->Kd = state->Kp*Tu/8;
            chSysUnlock();
          }
        }

        RAD_DEBUG_PRINTF("Autotune: heating: bias=%d, d=%d\n", bias, d);
        outputSet(cht->heating_pwm_id, (bias + d) / 2);
        total_cycles++;
        pvLow = pv;

        chSysLock();
        state->tuning_cycle_remains--;
        chSysUnlock();
      }
    }

    if (pv > target + 20)
    {
      printerEstop(L_TEMPERATURE_AUTOTUNE_OVERHEATED);
      break;
    }

    if (chTimeNow() - now > S2ST(2))
    {
      RAD_DEBUG_PRINTF("Autotune: pwm=%d, pv=%.1f, target=%.1f, output=%d\n",
          cht->heating_pwm_id,
          pv, target,
          outputGet(cht->heating_pwm_id));
      now = chTimeNow();
    }

    if(((chTimeNow() - t1) + (chTimeNow() - t2)) > S2ST(1200)) {
      printerEstop(L_TEMPERATURE_AUTOTUNE_TIMEOUT);
      break;
    }

    if (state->tuning_cycle_remains == 0)
    {
      RAD_DEBUG_PRINTF("Autotune: done: Kp=%f, Ki=%f, Kd=%f\n", state->Kp, state->Ki, state->Kd);
      outputSet(cht->heating_pwm_id, 0);
      return;
    }

    if (printerIsEstopped())
    {
      break;;
    }
    chThdSleepMilliseconds(50);
  } // while(1)

  chSysLock();
  *state = backup_state;
  chSysUnlock();
}

static void temperature_pid_loop(uint8_t temp_id, float sv, float pv)
{
  RadTemp* cht = &machine.temperature.devices[temp_id];
  if (cht->heating_pwm_id < 0)
    return;

  RadTempPidState* state = &pid_states[temp_id];
  if (state->tuning_cycle_remains > 0)
    return;

  int32_t limit = 255;

  state->error = sv - pv;
  float pTerm = state->Kp * state->error;
  state->i_error += state->error;
  state->i_error = fmax(fmin(state->i_error, limit / state->Ki), 0);
  // TODO: i_error minimum is 0? or negative?
  float iTerm = state->Ki * state->i_error;
  float dTerm = state->d_state =
      state->Kd * (pv - state->d_pv) * 0.05 + state->d_state * 0.95;
  state->d_pv = pv;

  uint8_t output = fmin(fmax(pTerm + iTerm - dTerm, 0), limit);
  outputSet(cht->heating_pwm_id, output);
}

/** @} */
