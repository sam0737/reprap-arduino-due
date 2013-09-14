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
  { .adc_id = 1, .heating_pwm_id = 2, .converter = adccType1 },
  { .adc_id = 2, .heating_pwm_id = 3, .converter = adccType1 },
  { .adc_id = 0, .heating_pwm_id = 1, .converter = adccBedConverter },
  { .adc_id = 3, .cooling_pwm_id = 4, .converter = adccSAM3XATempSensor }
};

MAKE_LINEAR_FORWARD_KINEMATICS(kForward);
MAKE_LINEAR_INVERSE_KINEMATICS(kInverse);

machine_t machine =
{
    .power = {
        .always_on = 0
    },
    .kinematics = {
        .type = KINEMATICS_Linear,
        .forward_kinematics = kForward,
        .inverse_kinematics = kInverse,
        .max_traj_acceleration = 3000,
        .axis_count = RAD_NUMBER_AXES,
        .axes = (RadAxis[]) {
          { .name = AXIS_X, .type = AXIS_TYPE_Linear, .min_limit = -15, .max_limit = 175 },
          { .name = AXIS_Y, .type = AXIS_TYPE_Linear, .min_limit = 0, .max_limit = 255 },
          { .name = AXIS_Z, .type = AXIS_TYPE_Linear, .min_limit = 0, .max_limit = 103.5 },
        },
        .joint_count = RAD_NUMBER_JOINTS,
        .joints = (RadJoint[]) {
          {
            .stepper_id = 0, .min_endstop_id = 0, .max_endstop_id = -1,
            .min_limit = -15, .max_limit = 175,
            .max_velocity = 400, .max_acceleration = 800, .scale = 100, //45.7142,
            .home_search_vel = -75, .home_latch_vel = 2,
            .home_sequence = 1
          },
          {
            .stepper_id = 1, .min_endstop_id = 1, .max_endstop_id = -1,
            .min_limit = 0, .max_limit = 255,
            .max_velocity = 400, .max_acceleration = 2500, .scale = 100, //45.7142,
            .home_search_vel = -75, .home_latch_vel = 2,
            .home_sequence = 1
          },
          {
            .stepper_id = 2, .min_endstop_id = -1, .max_endstop_id = 2,
            .min_limit = 0, .max_limit = 103.5,
            .max_velocity = 20, .max_acceleration = 100, .scale = 100, //45.7142,
            .home_search_vel = 20, .home_latch_vel = 2,
            .home_sequence = 0
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
        .count = 4,
        .devices = temps,
    },
    .extruder = {
        .count = RAD_NUMBER_EXTRUDERS,
        .devices = (RadExtruder[]) {
          {
            .temp = &temps[0],
            .stepper_id = 3,
            .max_velocity = 50,
            .max_acceleration = 1000,
            .max_retract_velocity = 50,
            .max_retract_acceleration = 1000,
            .scale = 100
          },
          {
            .temp = &temps[1],
            .stepper_id = 4,
            .max_velocity = 50,
            .max_acceleration = 1000,
            .max_retract_velocity = 50,
            .max_retract_acceleration = 1000,
            .scale = 215
          },
        }
    },
    .heated_bed = {
        .count = 1,
        .devices = (RadHeatedBed[]) {
          { .temp = &temps[2] },
        }
    },
    .temp_monitor = {
        .count = 1,
        .devices = (RadTemp*[]) {
          &temps[3]
        }
    },
    .fan = {
        .count = 1,
        .devices = (RadFan[]) {
          { .pwm_id = 5 }
        }
    }
};

/** @} */
