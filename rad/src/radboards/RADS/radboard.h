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

 ---

 A special exception to the GPL can be applied should you wish to distribute
 a combined work that includes RAD, without being obliged to provide
 the source code for any proprietary components. See the file exception.txt
 for full details of how and when the exception can be applied.
 */

/**
 * @file    RADS/radboard.h
 * @brief   radboard configuration header.
 *
 * @addtogroup RAD_BOARD
 * @{
 */

#ifndef _RADBOARD_H_
#define _RADBOARD_H_

#if HAL_USE_MSD
#include "usb_msd.h"
#endif
#include "usbcfg.h"

extern MMCDriver MMCD1;

/**
 * @brief Board identifier.
 */
#define RADBOARD_NAME "RADS"
#define RADBOARD_RADS         TRUE

/**
 * @brief System clock in Hz
 */
#define SYSTEM_CLOCK 84000000

/**
 * @brief Define if PAL pad change is atomic in this architecture
 */
#define PAL_PAD_IS_ATOMIC     TRUE

/**
 * @brief Define the R2 value of the thermistor circuit
 */
#define TEMP_R2 560

/**
 * @brief Define number of inputs
 */
#define RAD_NUMBER_INPUTS     1

/**
 * @brief Define number of endstops
 */
#define RAD_NUMBER_ENDSTOPS   3

/**
 * @brief Define number of stepper outputs
 */
#define RAD_NUMBER_STEPPERS   5

/**
 * @brief Define number of PWM outputs
 */
#define RAD_NUMBER_OUTPUTS    7

/**
 * @brief Define number of ADC inputs
 */
#define RAD_NUMBER_ADCS       1

/**
 * @brief Define board support software display contrast control
 *        Implementation might need to determine based on display model
 */
#ifndef RAD_DISPLAY_CONTRAST_SUPPORT
  #if GFX_USE_TDISP
    #define RAD_DISPLAY_CONTRAST_SUPPORT TRUE
  #endif
#endif

/**
 * Default hardware connections
 */
#define RADBOARD_ENDSTOP_X            0
#define RADBOARD_ENDSTOP_Y            1
#define RADBOARD_ENDSTOP_Z            2

#define RADBOARD_X_STEPPER            0
#define RADBOARD_Y_STEPPER            1
#define RADBOARD_Z_STEPPER            2
#define RADBOARD_EXTRUDER_1_STEPPER   3
#define RADBOARD_EXTRUDER_2_STEPPER   4

#define RADBOARD_EXTRUDER_1_OUTPUT    2
#define RADBOARD_EXTRUDER_2_OUTPUT    3
#define RADBOARD_BED_OUTPUT           1

#define RADBOARD_EXTRUDER_1_TEMP_ADC  1
#define RADBOARD_EXTRUDER_2_TEMP_ADC  2
#define RADBOARD_BED_TEMP_ADC         0
#define RADBOARD_SYSTEM_TEMP_ADC      3

#endif /* _RADBOARD_H_ */
