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
 * @file    src/radhal.h
 * @brief   RAD Hardware Abstraction Layer (Machine config) header
 *
 * @addtogroup RAD_HAL_MACHINE
 * @{
 */

#ifndef _RADHAL_MACHINE_H_
#define _RADHAL_MACHINE_H_

#include "hal.h"
#include "chevents.h"

#include "radadc_converter.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef struct {
  uint8_t             adcId;
  uint8_t             heatingPwmId;
  uint8_t             coolingPwmId;
  adcconverter_t      converter;
  /* End of the configuration fields.*/
  float               sv;
  float               pv;
} RadTempChannel;

typedef struct {
  uint8_t             pwmId;
} RadFanChannel;

typedef struct {
  struct {
    uint8_t           alwaysOn:1;
  } power;
  struct {
    uint8_t           count;
    RadTempChannel    *channels;
  } extruders;
  struct {
    uint8_t           count;
    RadTempChannel    *channels;
  } heatedBeds;
  struct {
    uint8_t           count;
    RadTempChannel    *channels;
  } tempMonitors;
  struct {
    uint8_t           count;
    RadFanChannel     *channels;
  } fans;
} machine_t;

/**
 * @brief Machine configuration
 */
extern const machine_t machine;

#include "machine.h"

#endif  /* _RADHAL_MACHINE_H_ */
/** @} */
