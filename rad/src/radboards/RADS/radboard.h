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

#include "usb_msd.h"
#include "usbcfg.h"

extern MMCDriver MMCD1;

/**
 * @brief Board identifier.
 */
#define RADBOARD_NAME "RADS"

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
 * @brief Define number of stepper output
 */
#define RAD_NUMBER_STEPPERS 5

/**
 * @brief Define board support software display contrast control
 *        Implementation might need to determine based on display model
 */
#ifndef RAD_DISPLAY_CONTRAST_SUPPORT
  #if GFX_USE_TDISP
    #define RAD_DISPLAY_CONTRAST_SUPPORT TRUE
  #endif
#endif

#endif /* _RADBOARD_H_ */
