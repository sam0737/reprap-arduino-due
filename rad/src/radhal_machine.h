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

#include "machine.h"
#include "temperature_converter.h"
#include "kinematics.h"
#include "planner.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef enum {
  AXIS_None = '_',
  AXIS_X = 'X', AXIS_Y = 'Y', AXIS_Z = 'Z',
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

struct machine_t;

typedef void (*forward_kinematics_t)(const PlannerPhysicalPosition*, PlannerVirtualPosition*);
typedef void (*inverse_kinematics_t)(const PlannerVirtualPosition*, PlannerPhysicalPosition*);
typedef float (*traj_max_feedrate_t)(struct machine_t machine);

typedef enum {
  LIMIT_Normal = 0,
  LIMIT_MinHit = 1,
  LIMIT_MaxHit = 2
} RadLimitState;

typedef struct {
  uint8_t             stepper_id;
  int8_t              min_endstop_id;
  int8_t              max_endstop_id;

  float               min_limit;
  float               max_limit;

  float               max_speed;
  float               max_acceleration;
  float               scale;

  float               home_search_vel;
  float               home_latch_vel;
  /*
   * @brief Homing sequence. Starts with 0, must be < number of joints
   *        If this is -1, the joint is always home when commended
   *        even if user has not specified so - useful in Delta.
   */
  int8_t              home_sequence;
  /*
   * @brief The G28 homing axis code that this joints represents.
   */
  RadAxisName         home_axis_name;
} RadJoint;

typedef struct {
  bool_t              active_low;
} RadEndstopConfig;

typedef struct {
  uint8_t             adc_id;
  int8_t              heating_pwm_id;
  int8_t              cooling_pwm_id;
  adcconverter_t      converter;
} RadTemp;

typedef struct {
  uint8_t             temp_id;
  uint8_t             stepper_id;
  float               max_speed;
  float               max_acceleration;
  float               max_retract_speed;
  float               max_retract_acceleration;
  float               scale;
  volatile struct {
    float             pos;
  } state;
} RadExtruder;

typedef struct {
  uint8_t             temp_id;
} RadHeatedBed;

typedef struct {
  uint8_t             temp_id;
} RadTempMonitor;

typedef struct {
  uint8_t             pwm_id;
} RadFan;

typedef struct {
  uint8_t             enabled;
  uint8_t             input_id;
} RadUiInput;

typedef struct {
  float               contrast;
  RadUiInput          generic_wheel;
  RadUiInput          up_button;
  RadUiInput          down_button;
  RadUiInput          left_button;
  RadUiInput          right_button;
  RadUiInput          back_button;
  RadUiInput          enter_button;
} RadUiSettings;

typedef struct machine_t {
  struct {
    /**
     * @brief   Always on and disable software controlled PSU
     * @details If the hardware support software controlled PSU, should the
     *          machine be always on and disable all related features.
     */
    bool_t           always_on:1;
  } power;
  struct {
    RadKinematicsType type;
    forward_kinematics_t  forward_kinematics;
    inverse_kinematics_t  inverse_kinematics;
    traj_max_feedrate_t   traj_max_feedrate;
    RadAxis           *axes;
    RadJoint          *joints;
  } kinematics;
  struct {
    uint8_t           count;
    RadEndstopConfig  *configs;
  } endstop_config;
  struct {
    RadTemp           *devices;
  } temperature;
  struct {
    RadExtruder       *devices;
  } extruder;
  struct {
    uint8_t           count;
    RadHeatedBed      *devices;
  } heated_bed;
  struct {
    uint8_t           count;
    RadTempMonitor    *devices;
  } temp_monitor;
  struct {
    uint8_t           count;
    RadFan            *devices;
  } fan;
  RadUiSettings      ui;
} machine_t;

/**
 * @brief Machine configuration
 */
extern const machine_t machine;

#ifndef RAD_NUMBER_AXES
#error "Please define RAD_NUMBER_AXES in machine.h"
#endif

#ifndef RAD_NUMBER_JOINTS
#error "Please define RAD_NUMBER_JOINTS in machine.h"
#endif

#ifndef RAD_NUMBER_EXTRUDERS
#error "Please define RAD_NUMBER_EXTRUDERS in machine.h"
#endif

#ifndef RAD_NUMBER_TEMPERATURES
#error "Please define RAD_NUMBER_TEMPERATURES in machine.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
  char machineGetAxisName(RadAxisName name);
#ifdef __cplusplus
}
#endif

#endif  /* _RADHAL_MACHINE_H_ */
/** @} */
