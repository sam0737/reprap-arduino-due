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
 * @file    input.c
 * @brief   Input management
 *
 * @addtogroup INPUT
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "input.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waInput, 128);

static RadInputState states[RAD_NUMBER_INPUTS];

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static msg_t threadInput(void *arg) {
  (void)arg;
  chRegSetThreadName("input");

  while (TRUE)
  {
    chThdSleepMilliseconds(10);
#if RAD_NUMBER_INPUTS > 0
    for (uint8_t i = 0; i < RAD_NUMBER_INPUTS; i++)
    {
      if (states[i].is_enabled && radboard.input.channels[i].fetcher)
      {
        chSysLock();
        radboard.input.channels[i].fetcher(&radboard.input.channels[i].config, &states[i]);
        chSysUnlock();
      }
    }
#endif
  }
  return 0;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void inputInit(void)
{
  if (RAD_NUMBER_INPUTS > 0)
    chThdCreateStatic(waInput, sizeof(waInput), NORMALPRIO + 1, threadInput, NULL);
}

void inputEnable(uint8_t input_id)
{
  states[input_id].is_enabled = 1;
}

RadInputValue inputGet(uint8_t input_id)
{
  RadInputValue value;
  if (input_id >= RAD_NUMBER_INPUTS) {
    memset(&value, 0, sizeof(value));
    return value;
  }
  chSysLock();
  value = radboard.input.channels[input_id].processor(&states[input_id]);
  chSysUnlock();
  return value;
}

/** @} */
