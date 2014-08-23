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
 * @file    generic/machine.h
 * @brief   machine configuration header.
 *
 * @addtogroup RAD_BOARD
 * @{
 */

#ifndef _MACHINE_H_
#define _MACHINE_H_

#include "localization/english.h"

/**
 * @brief Board identifier.
 */
#define MACHINE_NAME "Generic"

#define RAD_NUMBER_JOINTS 3
#define RAD_NUMBER_AXES 3
#define RAD_NUMBER_EXTRUDERS      2 // (RAD_NUMBER_STEPPERS - 3)

#ifdef RADBOARD_BED_TEMP_ADC
#define RAD_NUMBER_TEMPERATURES_HAS_BED 1
#else
#define RAD_NUMBER_TEMPERATURES_HAS_BED 0
#endif

#ifdef RADBOARD_SYSTEM_TEMP_ADC
#define RAD_NUMBER_TEMPERATURES_HAS_SYSTEM 1
#else
#define RAD_NUMBER_TEMPERATURES_HAS_SYSTEM 0
#endif

#define RAD_NUMBER_TEMPERATURES (\
  RAD_NUMBER_EXTRUDERS + \
  RAD_NUMBER_TEMPERATURES_HAS_BED + \
  RAD_NUMBER_TEMPERATURES_HAS_SYSTEM)

#endif /* _MACHINE_H_ */
