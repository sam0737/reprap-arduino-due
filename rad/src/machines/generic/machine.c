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

const machine_t machine =
{
    .power = {
        .alwaysOn = 0
    },
    .extruders = {
        .count = 2,
        .channels = (RadTempChannel[]) {
          { .adcId = 2, .heatingPwmId = 2, .converter = adccType1 },
          { .adcId = 3, .heatingPwmId = 3, .converter = adccType1 }
        }
    },
    .heatedBeds = {
        .count = 1,
        .channels = (RadTempChannel[]) {
          { .adcId = 1, .heatingPwmId = 1, .converter = adccBedConverter }
        }
    },
    .tempMonitors = {
        .count = 1,
        .channels = (RadTempChannel[]) {
          { .adcId = 4, .coolingPwmId = 4, .converter = adccSAM3XATempSensor }
        }
    },
    .fans = {
        .count = 1,
        .channels = (RadFanChannel[]) {
          { .pwmId = 5 }
        }
    }
};

/** @} */
