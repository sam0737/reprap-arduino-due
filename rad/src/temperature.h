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
 * @file    temperature.h
 * @brief   Temperature header
 *
 * @addtogroup TEMPERATURE
 * @{
 */
#ifndef _RAD_TEMPERATURE_H_
#define _RAD_TEMPERATURE_H_

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

#if !HAL_USE_ADC
typedef uint16_t adcsample_t;
#endif

typedef struct {
  float   Kp;
  float   Ki;
  float   Kd;
} RadTempConfig;

typedef struct {
  float               sv;
  float               pv;
  adcsample_t         raw;
  bool_t              is_heating;
  systime_t           target_reached_at;
  bool_t              target_reached;
} RadTempState;

typedef struct {
  RadTempConfig config;
  float   error;
  float   i_error;
  float   d_state;
  float   d_pv;
  uint8_t tuning_cycle_remains;
} RadTempPidState;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void temperatureInit(void);
  void temperatureSet(uint8_t temp_id, float temp);
  void temperatureAllZero(void);
  RadTempState temperatureGet(uint8_t temp_id);
  void temperatureAutoTune(uint8_t temp_id, float target, uint8_t total_cycles);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_ADC_H_ */

/** @} */
