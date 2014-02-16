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
 * @file    temperature_converter.c
 * @brief   Temperature Converters
 *
 * @addtogroup TEMPERATURE_CONVERTER
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "temperature_converter.h"

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

#ifdef TEMP_R2
// 100k thermistor - best choice for EPCOS 100k (B57540G0104F000)
MAKE_THERMISTOR_CONVERTER(adccType1, TEMP_R2, 100000, 25, 4066);
#endif

float adccSAM3XATempSensor(const adcsample_t sample, const uint8_t resolution)
{
  return
      (
        ((float)sample / (1 << resolution)) * 3.3f // Voltage
        - 0.8f // 27deg base voltage
      ) / 0.00265 // mV per degree
      + 27;
}

float adccDummy(const adcsample_t sample, const uint8_t resolution)
{
  (void) resolution;
  return sample;
}

/** @} */
