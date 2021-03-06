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
 * @file    src/rad.h
 * @brief   RAD header.
 *
 * @addtogroup RAD
 * @{
 */

#ifndef _RAD_H_
#define _RAD_H_

#include "math.h"
#include "mini_printf.h"

#include "power.h"
#include "beeper.h"
#include "input_hardware.h"
#include "input.h"
#include "output.h"
#include "endstop.h"
#include "temperature.h"
#include "raddebug.h"
#include "ui.h"
#include "storage.h"
#include "radmath.h"

#include "radhal_radboard.h"
#include "radhal_machine.h"

#include "gcode_definition.h"
#include "printer.h"
#include "gcode.h"

#include "stepper.h"
#include "planner.h"
#include "planner_queue.h"
#include "radpex.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void radInit(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_H_ */

/** @} */
