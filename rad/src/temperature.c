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

#include "temperature_pid.h"

#if HAL_USE_ADC
#include "temperature_core_real.h"
#else
#include "temperature_core_simulated.h"
#endif

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void temperatureInit()
{
  temperature_core_init();
  chThdCreateStatic(waTemp, sizeof(waTemp), NORMALPRIO, threadTemp, NULL);
}
/** @} */
