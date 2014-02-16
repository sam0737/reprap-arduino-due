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
 * @file    temperature_core_simulated.h
 * @brief   Temperature Core (Simulated)
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

static WORKING_AREA(waTemp, 64);
static RadTempState temperatures[RAD_NUMBER_TEMPERATURES];

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static msg_t threadTemp(void *arg) {
  (void)arg;
  uint8_t i, j, k, c;

  chRegSetThreadName("temp");

  while (TRUE) {
    chThdSleepMilliseconds(50);

    for (k = 0; k < RAD_NUMBER_TEMPERATURES; k++) {
      RadTemp *cht = &machine.temperature.devices[k];
      if (cht->heating_pwm_id >= 0) {
        RadTempState *s = &temperatures[k];

        chSysLock();
        // Simulated output and temperature
        uint8_t duty = outputGet(cht->heating_pwm_id);
        if (65535 - (uint16_t) duty >= s->raw) {
          s->raw += duty;
        } else {
          s->raw = 65535;
        }
        s->raw = s->raw * 0.99;
        s->pv = s->raw / (float)(255 * 100 / 1) * 300;
        float sv = s->sv;
        chSysUnlock();
        temperature_pid_loop(k, sv, s->pv);
      }
    }
  }
  return 0;
}

static void temperatureSetI(uint8_t temp_id, float temp)
{
  temperatures[temp_id].sv = temp;
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
    temp.pv = -1;
    temp.raw = 0;
    return temp;
  }

  chSysLock();
  temp = temperatures[temp_id];
  chSysUnlock();
  return temp;
}

void temperature_core_init()
{
}
/** @} */
