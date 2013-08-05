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
 * @file    radadc_converter.h
 * @brief   RAD ADC Converters header
 *
 * @addtogroup RADADC_CONVERTER
 * @{
 */
#ifndef _RAD_ADC_CONVERTER_H_
#define _RAD_ADC_CONVERTER_H_

#include <math.h>

/*===========================================================================*/
/* Macro definitions                                                         */
/*===========================================================================*/

/**
 * @brief Generate converter function for NTC thermistor.
 *        NAME is the function name,
 *        R2 is the onboard resistor,
 *        BETA, R0, T0 are thermisitor parameters (T0 is in celsius)
 */
#define MAKE_THERMISTOR_CONVERTER(NAME, R2, R0, T0, BETA)                   \
float (NAME)(const adcsample_t sample, const uint8_t resolution)            \
{                                                                           \
  uint16_t limit = (1 << (resolution - 8));                                 \
  if (sample <= limit) return 999;                                          \
  adcsample_t inv_sample = (1 << resolution) - 1 - sample;                  \
  if (inv_sample <= limit) return 999;                                      \
  return -273.15 +                                                          \
    BETA / log(                                                             \
      sample * (R2) / inv_sample /                                          \
      ((R0) * exp(-(BETA) / (T0 + 273.15)))                                 \
    );                                                                      \
}

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

/**
 * @brief Conversion function for converting sample into temperature result
 */
typedef float (*adcconverter_t)(const adcsample_t sample, const uint8_t resolution);

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  float adccSAM3XATempSensor(const adcsample_t sample, const uint8_t resolution);
  float adccDummy(const adcsample_t sample, const uint8_t resolution);
#ifdef TEMP_R2
  float adccType1(const adcsample_t sample, const uint8_t resolution);
#endif
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_ADC_CONVERTER_H_ */

/** @} */
