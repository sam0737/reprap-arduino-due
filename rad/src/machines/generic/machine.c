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
 * @file    generic/machine.c
 * @brief   Generic machine definition.
 *
 * @addtogroup MACHINE
 * @{
 */

#include "rad.h"

MAKE_THERMISTOR_CONVERTER(adccBedConverter, TEMP_R2, 4013, 101, 4675);

static RadTemp temps[] = {
  { .adc_id = 1, .heating_pwm_id = 2, .cooling_pwm_id = -1, .converter = adccType1 },
  { .adc_id = 2, .heating_pwm_id = 3, .cooling_pwm_id = -1, .converter = adccType1 },
  { .adc_id = 0, .heating_pwm_id = 1, .cooling_pwm_id = -1, .converter = adccBedConverter },
  { .adc_id = 3, .heating_pwm_id = -1, .cooling_pwm_id = 4, .converter = adccSAM3XATempSensor }
};

machine_t machine;

MAKE_LINEAR_FORWARD_KINEMATICS(kForward);
MAKE_LINEAR_INVERSE_KINEMATICS(kInverse);
MAKE_CARTESIAN_MAX_FEEDRATE(kTrajMaxFeedrate);

machine_t machine =
{
    .power = {
        .always_on = 0
    },
    .kinematics = {
        .type = KINEMATICS_Linear,
        .forward_kinematics = kForward,
        .inverse_kinematics = kInverse,
        .traj_max_feedrate = kTrajMaxFeedrate,
        .axes = (RadAxis[]) {
          { .name = AXIS_X, .type = AXIS_TYPE_Linear, .min_limit = -15, .max_limit = 175 },
          { .name = AXIS_Y, .type = AXIS_TYPE_Linear, .min_limit = 0, .max_limit = 255 },
          { .name = AXIS_Z, .type = AXIS_TYPE_Linear, .min_limit = 0, .max_limit = 103.5 },
        },
        .joints = (RadJoint[]) {
          {
            .stepper_id = 0, .min_endstop_id = 0, .max_endstop_id = -1,
            .min_limit = -15, .max_limit = 175,
            .max_speed = 100, .max_acceleration = 100, .scale = 100, //45.7142,
            // .max_speed = 400, .max_acceleration = 800, .scale = 100, //45.7142,
            .home_search_vel = -75, .home_latch_vel = 2,
            .home_sequence = 1, .home_axis_name = AXIS_X
          },
          {
            .stepper_id = 1, .min_endstop_id = 1, .max_endstop_id = -1,
            .min_limit = 0, .max_limit = 255,
            .max_speed = 100, .max_acceleration = 100, .scale = 100, //45.7142,
            // .max_speed = 400, .max_acceleration = 2500, .scale = 100, //45.7142,
            .home_search_vel = -75, .home_latch_vel = 2,
            .home_sequence = 1, .home_axis_name = AXIS_Y
          },
          {
            .stepper_id = 2, .min_endstop_id = -1, .max_endstop_id = 2,
            .min_limit = 0, .max_limit = 103.5,
            .max_speed = 20, .max_acceleration = 100, .scale = 100, //45.7142,
            .home_search_vel = 20, .home_latch_vel = 2,
            .home_sequence = 0, .home_axis_name = AXIS_Z
          }
        },
    },
    .endstop_config = {
        .count = 3,
        .configs = (RadEndstopConfig[]) {
          { .active_low = FALSE },
          { .active_low = FALSE },
          { .active_low = FALSE }
        }
    },
    .temperature = {
        .devices = temps,
    },
    .extruder = {
        .devices = (RadExtruder[]) {
          {
            .temp_id = 0,
            .stepper_id = 3,
            .max_speed = 50,
            .max_acceleration = 1000,
            .max_retract_speed = 50,
            .max_retract_acceleration = 1000,
            .scale = 100
          },
          {
            .temp_id = 1,
            .stepper_id = 4,
            .max_speed = 50,
            .max_acceleration = 1000,
            .max_retract_speed = 50,
            .max_retract_acceleration = 1000,
            .scale = 215
          },
        }
    },
    .heated_bed = {
        .count = 1,
        .devices = (RadHeatedBed[]) {
          { .temp_id = 2 },
        }
    },
    .temp_monitor = {
        .count = 1,
        .devices = (RadTempMonitor[]) {
          { .temp_id = 3 }
        }
    },
    .fan = {
        .count = 1,
        .devices = (RadFan[]) {
          { .pwm_id = 5 }
        }
    },
    .ui = {
        .contrast = 0.5
    }
};

/** @} */
