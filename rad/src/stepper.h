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
 * @file    stepper.h
 * @brief   Stepper header
 *
 * @addtogroup STEPPER
 * @{
 */
#ifndef _RAD_STEPPER_H_
#define _RAD_STEPPER_H_

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

typedef struct {
  bool_t              stopped;
  bool_t              homed;
  RadLimitState       limit_state;
  RadLimitState       changed_limit_state;
  RadLimitState       base_limit_state;
  int32_t             limit_step;
  float               pos;
} RadJointState;

typedef struct {
  RadJointState joints[RAD_NUMBER_AXES];
} RadJointsState;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void stepperInit(void);
  void stepperSetHome(uint8_t joint_id, int32_t home_step, float home_pos);
  RadJointsState stepperGetJointsState(void);
  void stepperSetLimitState(uint8_t joint_id, RadLimitState limit);
  void stepperResetOldLimitState(uint8_t joint_id);
  void stepperClearStopped(uint8_t joint_id);
  void stepperSetHomed(uint8_t joint_id);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_STEPPER_H_ */

/** @} */
