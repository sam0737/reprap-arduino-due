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
 * @file    power.c
 * @brief   power management
 *
 * @addtogroup POWER
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "power.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/
static Mutex mutex;

/**
 * State of the power supply
 */
static bool_t psu_state;

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void powerInit(void)
{
  chMtxInit(&mutex);
  if (!palHasSig(radboard.power.psu_on)) {
    psu_state = TRUE;
    return;
  }
  if (machine.power.always_on) {
    pexEnableSig(radboard.power.psu_on);
    psu_state = TRUE;
  } else {
    pexDisableSig(radboard.power.psu_on);
    psu_state = FALSE;
  }
  palSetSigMode(radboard.power.psu_on, PAL_MODE_OUTPUT_PUSHPULL);
}

void powerPsuOn(void)
{
  if (!palHasSig(radboard.power.psu_on) || machine.power.always_on)
    return;

  chMtxLock(&mutex);
  if (!psu_state)
  {
    psu_state = TRUE;
    pexEnableSig(radboard.power.psu_on);

    // Wait for power to stable
    chThdSleepMilliseconds(200);
  }
  chMtxUnlock();
}

void powerPsuOff(void)
{
  if (!palHasSig(radboard.power.psu_on) || machine.power.always_on)
    return;

  chMtxLock(&mutex);
  if (psu_state)
  {
    psu_state = FALSE;
    pexDisableSig(radboard.power.psu_on);
  }
  chMtxUnlock();
}

uint8_t powerIsPsuOn(void)
{
  chMtxLock(&mutex);
  bool_t state = psu_state;
  chMtxUnlock();
  return state;
}

/** @} */
