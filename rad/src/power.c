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

/**
 * State of the power supply
 */
static volatile uint8_t psuState;

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void powerInit(void)
{
  if (!palHasSig(radboard.power.psuOn))
    return;
  if (machine.power.alwaysOn) {
    pexEnableSig(radboard.power.psuOn);
    psuState = 1;
  } else {
    pexDisableSig(radboard.power.psuOn);
    psuState = 0;
  }
  palSetSigMode(radboard.power.psuOn, PAL_MODE_OUTPUT_PUSHPULL);
}

void powerPsuOn(void)
{
  if (!palHasSig(radboard.power.psuOn) || machine.power.alwaysOn)
    return;

  psuState = 1;
  pexEnableSig(radboard.power.psuOn);
}

void powerPsuOff(void)
{
  if (!palHasSig(radboard.power.psuOn) || machine.power.alwaysOn)
    return;

  psuState = 0;
  pexDisableSig(radboard.power.psuOn);
}

uint8_t powerIsPsuOn(void)
{
  return psuState;
}

/** @} */
