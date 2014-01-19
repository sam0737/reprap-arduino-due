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
 * @file    kinematics.h
 * @brief   Kinematics formula
 *
 * @addtogroup KINEMATICS
 * @{
 */
#ifndef _RAD_KINEMATICS_H_
#define _RAD_KINEMATICS_H_

#include <math.h>

/*===========================================================================*/
/* Macro definitions                                                         */
/*===========================================================================*/

/**
 * @brief Generate forward kinematics for simple linear motion
 *        (joint coordinate to axis coordinate)
 * @param[in] name    The name of the function generated
 */
#define MAKE_LINEAR_FORWARD_KINEMATICS(NAME)                              \
void (NAME)(const PlannerPhysicalPosition *p, PlannerVirtualPosition *v)  \
{                                                                         \
  uint8_t i;                                                              \
  for (i = 0; i < RAD_NUMBER_AXES; i++) v->axes[i] = p->joints[i];        \
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++)                              \
    v->extruders[i] = p->extruders[i];                                    \
}                                                                         \

/**
 * @brief Generate inverse kinematics for simple linear motion
 *        (axis coordinate to joint coordinate)
 * @param[in] name    The name of the function generated
 */
#define MAKE_LINEAR_INVERSE_KINEMATICS(NAME)                              \
void (NAME)(const PlannerVirtualPosition *v, PlannerPhysicalPosition *p)  \
{                                                                         \
  uint8_t i;                                                              \
  for (i = 0; i < RAD_NUMBER_AXES; i++) p->joints[i] = v->axes[i];        \
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++)                              \
    p->extruders[i] = v->extruders[i];                                    \
}                                                                         \

#define MAKE_CARTESIAN_MAX_FEEDRATE(NAME)                                 \
float (NAME)(machine_t machine)                                           \
{                                                                         \
  float x = 0.0f;                                                         \
  uint8_t i;                                                              \
  for (i = 0; i < RAD_NUMBER_JOINTS; i++)                                 \
    x +=                                                                  \
        machine.kinematics.joints[i].max_speed *                          \
        machine.kinematics.joints[i].max_speed;                           \
  return sqrt(x);                                                         \
}                                                                         \

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _RAD_KINEMATICS_H_ */

/** @} */
