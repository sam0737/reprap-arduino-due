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
 * @file    temperature.c
 * @brief   Temperature
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

static RadTempPidState pid_states[RAD_NUMBER_TEMPERATURES];
static RadTempState temperatures[RAD_NUMBER_TEMPERATURES];

#include "temperature_pid.h"

#if HAL_USE_ADC
#include "temperature_core_real.h"
#else
#include "temperature_core_simulated.h"
#endif

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

static void temperatureSetI(uint8_t temp_id, float temp)
{
  RadTempState* t = &temperatures[temp_id];
  t->sv = temp;
  t->is_heating = t->sv > t->pv;
  t->target_reached = 0;
  t->target_reached_at = 0;
}

void temperatureSet(uint8_t temp_id, float temp)
{
  if (temp_id >= RAD_NUMBER_TEMPERATURES)
    return;

  chSysLock();
  temperatureSetI(temp_id, temp);
  chSysUnlock();
}

void temperatureAllZero(void)
{
  chSysLock();
  for (uint8_t i = 0; i < RAD_NUMBER_TEMPERATURES; i++)
    temperatureSetI(i, 0);
  chSysUnlock();
}

RadTempState temperatureGet(uint8_t temp_id)
{
  RadTempState temp;

  if (temp_id >= RAD_NUMBER_TEMPERATURES)
  {
    memset(&temp, 0, sizeof(temp));
    return temp;
  }

  chSysLock();
  temp = temperatures[temp_id];
  if (!temp.target_reached && temp.target_reached_at)
  {
    systime_t residency_time = machine.temperature.devices[temp_id].residency_time;
    if (residency_time == 0)
      residency_time = S2ST(10);
    temp.target_reached = chTimeNow() - temp.target_reached_at > residency_time;
  }
  chSysUnlock();
  return temp;
}

void temperatureInit()
{
  for (uint8_t i = 0; i < RAD_NUMBER_TEMPERATURES; i++)
  {
    pid_states[i].config = machine.temperature.devices[i].config;
  }
  temperature_core_init();
  chThdCreateStatic(waTemp, sizeof(waTemp), NORMALPRIO, threadTemp, NULL);
}
/** @} */
