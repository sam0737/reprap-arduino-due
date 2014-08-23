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
#if defined(RADBOARD_EXTRUDER_1_TEMP_ADC) && RAD_NUMBER_EXTRUDERS >= 1
  {
    .adc_id = RADBOARD_EXTRUDER_1_TEMP_ADC,
    .heating_pwm_id = RADBOARD_EXTRUDER_1_OUTPUT,
    .cooling_pwm_id = -1,
    .converter = adccType1,
    .config = { .Kp=2.8, .Ki=0.55, .Kd=3.53 }
  },
#endif
#if defined(RADBOARD_EXTRUDER_2_TEMP_ADC) && RAD_NUMBER_EXTRUDERS >= 2
  {
    .adc_id = RADBOARD_EXTRUDER_2_TEMP_ADC,
    .heating_pwm_id = RADBOARD_EXTRUDER_2_OUTPUT,
    .cooling_pwm_id = -1,
    .converter = adccType1,
    .config = { .Kp=2.8, .Ki=0.55, .Kd=3.53 }
  },
#endif
#if defined(RADBOARD_EXTRUDER_3_TEMP_ADC) && RAD_NUMBER_EXTRUDERS >= 3
  {
    .adc_id = RADBOARD_EXTRUDER_3_TEMP_ADC,
    .heating_pwm_id = RADBOARD_EXTRUDER_3_OUTPUT,
    .cooling_pwm_id = -1,
    .converter = adccBedConverter,
    .config = { .Kp=2.8, .Ki=0.55, .Kd=3.53 }
  },
#endif
#ifdef RADBOARD_BED_TEMP_ADC
  {
    .adc_id = RADBOARD_BED_TEMP_ADC,
    .heating_pwm_id = RADBOARD_BED_OUTPUT,
    .cooling_pwm_id = -1,
    .converter = adccBedConverter,
    .config = { .Kp=2.8, .Ki=0.55, .Kd=3.53 }
  },
#endif
#ifdef RADBOARD_SYSTEM_TEMP_ADC
  {
    .adc_id = RADBOARD_SYSTEM_TEMP_ADC,
    .heating_pwm_id = -1,
    .cooling_pwm_id = -1,
    .converter = adccSAM3XATempSensor
  },
#endif
};

const machine_t machine;

MAKE_LINEAR_FORWARD_KINEMATICS(kForward);
MAKE_LINEAR_INVERSE_KINEMATICS(kInverse);
MAKE_CARTESIAN_MAX_FEEDRATE(kTrajMaxFeedrate);

const machine_t machine =
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
            .stepper_id = 0, .min_endstop_id = RADBOARD_ENDSTOP_X, .max_endstop_id = -1,
            .min_limit = -15, .max_limit = 175,
            //.max_speed = 100, .max_acceleration = 100, .scale = 100, //45.7142,
            .max_speed = 400, .max_acceleration = 800, .scale = 100, //45.7142,
            .home_search_vel = -20, .home_latch_vel = -2,
            .home_sequence = 1, .home_axis_name = AXIS_X
          },
          {
            .stepper_id = 1, .min_endstop_id = RADBOARD_ENDSTOP_Y, .max_endstop_id = -1,
            .min_limit = 0, .max_limit = 255,
            //.max_speed = 100, .max_acceleration = 100, .scale = 100, //45.7142,
            .max_speed = 400, .max_acceleration = 2500, .scale = 100, //45.7142,
            .home_search_vel = -20, .home_latch_vel = -2,
            .home_sequence = 1, .home_axis_name = AXIS_Y
          },
          {
            .stepper_id = 2, .min_endstop_id = -1, .max_endstop_id = RADBOARD_ENDSTOP_Z,
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
#if defined(RADBOARD_EXTRUDER_1_STEPPER) && RAD_NUMBER_EXTRUDERS >= 1
          {
            .temp_id = 0,
            .stepper_id = RADBOARD_EXTRUDER_1_STEPPER,
            .max_speed = 50,
            .max_acceleration = 1000,
            .max_retract_speed = 50,
            .max_retract_acceleration = 1000,
            .scale = 100
          },
#endif
#if defined(RADBOARD_EXTRUDER_2_STEPPER) && RAD_NUMBER_EXTRUDERS >= 2
          {
            .temp_id = 1,
            .stepper_id = RADBOARD_EXTRUDER_2_STEPPER,
            .max_speed = 50,
            .max_acceleration = 1000,
            .max_retract_speed = 50,
            .max_retract_acceleration = 1000,
            .scale = 215
          },
#endif
#if defined(RADBOARD_EXTRUDER_3_STEPPER) && RAD_NUMBER_EXTRUDERS >= 3
          {
            .temp_id = 2,
            .stepper_id = RADBOARD_EXTRUDER_3_STEPPER,
            .max_speed = 50,
            .max_acceleration = 1000,
            .max_retract_speed = 50,
            .max_retract_acceleration = 1000,
            .scale = 215
          },
#endif
        }
    },
    .heated_bed = {
        .count = 1,
        .devices = (RadHeatedBed[]) {
          { .temp_id = RAD_NUMBER_EXTRUDERS },
        }
    },
    .temp_monitor = {
        .count = 1,
        .devices = (RadTempMonitor[]) {
          { .temp_id = RAD_NUMBER_TEMPERATURES - 1 }
        }
    },
    .fan = {
        .count = 0,
        /*
        .devices = (RadFan[]) {
          { .pwm_id = 5 }
        }
        */
    },
    .ui = {
        .contrast = 0.5,
        .generic_wheel = { .enabled = 1, .input_id = 0 },
        .enter_button = { .enabled = 1, .input_id = 1 },
        .back_button = { .enabled = 1, .input_id = 3 }
    }
};

/** @} */
