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
 * @file    src/radhal_machine.h
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
#include "kinematics.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef enum {
  AXIS_None,
  AXIS_X, AXIS_Y, AXIS_Z,
  /* TODO: Support for angular axis
    AXIS_U, AXIS_V, AXIS_W,
    AXIS_A, AXIS_B, AXIS_C
  */
} RadAxisName;

typedef enum {
  AXIS_TYPE_Linear
  // TODO: Support for axis that loop around (angular)
} RadAxisType;

typedef struct {
  RadAxisName         name;
  RadAxisType         type;
  float               min_limit;
  float               max_limit;
} RadAxis;

typedef enum {
  KINEMATICS_Linear, KINEMATICS_Custom
} RadKinematicsType;

typedef void (*forward_kinematics_t)(float *joints, float *axes);
typedef void (*inverse_kinematics_t)(float *axes, float *joints);

typedef enum {
  LIMIT_Normal, LIMIT_MaxHit, LIMIT_MinHit
} RadLimitState;

typedef struct {
  RadLimitState       limit_state;
} RadJointState;

typedef struct {
  uint8_t             stepper_id;
  int8_t             min_endstop_id;
  int8_t             max_endstop_id;

  float               min_limit;
  float               max_limit;

  float               max_velocity;
  float               max_acceleration;
  float               scale;

  float               home_search_vel;
  float               home_latch_vel;
  float               home_final_vel;
  float               home_offset;
  float               home;
  uint8_t             home_sequence;

  volatile RadJointState state;
} RadJoint;

typedef struct {
  bool_t              active_low;
} RadEndstopConfig;

typedef struct {
  float               sv;
  float               pv;
} RadTempState;

typedef struct {
  uint8_t             adc_id;
  uint8_t             heating_pwm_id;
  uint8_t             cooling_pwm_id;
  adcconverter_t      converter;
  volatile RadTempState state;
} RadTemp;

typedef struct {
  RadTemp             *temp;

  uint8_t             stepper_id;
  float               max_velocity;
  float               max_acceleration;
  float               max_retract_velocity;
  float               max_retract_acceleration;
  float               scale;
} RadExtruder;

typedef struct {
  RadTemp             *temp;
} RadHeatedBed;

typedef struct {
  uint8_t             pwm_id;
} RadFan;

typedef struct {
  struct {
    /**
     * @brief   Always on and disable software controlled PSU
     * @details If the hardware support software controlled PSU, should the
     *          machine be always on and disable all related features.
     */
    uint8_t           always_on:1;
  } power;
  struct {
    RadKinematicsType type;
    forward_kinematics_t  forward_kinematics;
    inverse_kinematics_t  inverse_kinematics;
    float             max_traj_acceleration;
    uint8_t           axis_count;
    RadAxis           *axes;
    uint8_t           joint_count;
    RadJoint          *joints;
  } kinematics;
  struct {
    uint8_t           count;
    RadEndstopConfig  *configs;
  } endstop_config;
  struct {
    uint8_t           count;
    RadTemp           *devices;
  } temperature;
  struct {
    uint8_t           count;
    RadExtruder       *devices;
  } extruder;
  struct {
    uint8_t           count;
    RadHeatedBed      *devices;
  } heated_bed;
  struct {
    uint8_t           count;
    RadTemp           **devices;
  } temp_monitor;
  struct {
    uint8_t           count;
    RadFan            *devices;
  } fan;
} machine_t;

/**
 * @brief Machine configuration
 */
extern const machine_t machine;

#include "machine.h"

#ifndef RAD_NUMBER_AXES
#error "Please define RAD_NUMBER_AXES in machine.h"
#endif

#ifndef RAD_NUMBER_JOINTS
#error "Please define RAD_NUMBER_JOINTS in machine.h"
#endif

#ifndef RAD_NUMBER_EXTRUDERS
#error "Please define RAD_NUMBER_EXTRUDERS in machine.h"
#endif

#endif  /* _RADHAL_MACHINE_H_ */
/** @} */
