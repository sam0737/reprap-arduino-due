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
 * @file    src/radhal_board.h
 * @brief   RAD Hardware Abstraction Layer (Board config) header.
 *
 * @addtogroup RAD_HAL_RADBOARD
 * @{
 */

#ifndef _RADHAL_RADBOARD_H_
#define _RADHAL_RADBOARD_H_

#include "hal.h"
#include "chevents.h"

#include "radboard.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief Define a IO pin
 */
typedef struct {
  /**
   * @brief   The PIO port
   */
  ioportid_t  port;

  /**
   * @brief   The pin number
   */
  uint8_t     pin;
} pin_t;

typedef struct {
  pin_t       pin;
  uint8_t     active_low;
} signal_t;

typedef struct {
#if HAL_USE_PWM
  PWMDriver       *pwm;
  pwmchannel_t    channel;
#endif
  signal_t        signal;
} RadOutputChannel;

typedef struct {
  signal_t        enable;
  signal_t        step;
  signal_t        dir;
  int32_t         pos;
} RadStepperChannel;

typedef struct {
  pin_t           pin;
} RadEndstopChannel;

#if HAL_USE_ADC
typedef struct {
  ADCDriver       *adc;
  uint8_t         resolution;
  ADCConversionGroup  group_base;
  adcsample_t     *samples;
} RadAdcChannel;
#endif

typedef struct {
  /**
   * @brief Initialization routine called when startup
   */
  void        (*init)(void);

  /*******************************************
   * @brief Power management
   */
  struct {
    /**
     * @brief IO pin for ATX PSU On. Omit to disable this feature.
     * @note  Please define pin as active_low to turn the PSU on
     */
    signal_t        psu_on;
  } power;

  /*******************************************
   * Human machine interface
   */
  struct {
#if HAL_USE_PWM
    PWMDriver       *beeper_pwm;
    pwmchannel_t    beeper_channel;
#endif

    /**
     * @brief Host communication channel
     */
    BaseAsynchronousChannel  *comm_channel;

    BaseBlockDevice          *storage_device;
#if HAL_USE_MSD
    USBMassStorageDriver     *usb_msd;
#endif
  } hmi;

  /*******************************************
   * @brief PWM Output
   */
  struct {
    RadOutputChannel          *channels;
  } output;

  /*******************************************
   * @brief Stepper Output
   */
  struct {
    uint8_t                   count;
    signal_t                  main_enable;
#if HAL_USE_GPT
    GPTDriver                 *gpt;
    GPTConfig                 *gpt_config;
#endif
    RadStepperChannel         *channels;
  } stepper;

  /*******************************************
   * @brief Endstop Input
   */
  struct {
    uint8_t                   count;
    RadEndstopChannel         *channels;
  } endstop;

#if HAL_USE_ADC
  /*******************************************
   * @brief ADC Input
   */
  struct {
    uint8_t                   count;
    RadAdcChannel             *channels;
  } adc;
#endif

  /*******************************************
   * @brief Debug
   */
  struct {
    /**
     * @brief Heartbeat LED that flashes every half a second
     */
    pin_t     heartbeat_led;
    /**
     * @brief Shell channel
     */
    BaseAsynchronousChannel  *channel;
    /**
     * @brief Handler for reset
     */
    void      (*software_reset)(void);
    /**
     * @brief Handler for chip erase
     */
    void      (*erase)(void);
  } debug;
} radboard_t;

/**
 * @brief Board configuration
 */
extern const radboard_t radboard;

#ifndef SYSTEM_CLOCK
#error "Please define SYSTEM_CLOCK in radboard.h - System clock in Hz"
#endif

#ifndef RAD_NUMBER_STEPPERS
#error "Please define RAD_NUMBER_STEPPERS in radboard.h"
#endif

#ifndef RAD_NUMBER_OUTPUTS
#error "Please define RAD_NUMBER_OUTPUTS in radboard.h"
#endif

/*
 * Define noops for debug hook
 */
#ifndef RAD_DEBUG_PRINTF
#define RAD_DEBUG_PRINTF(...) do {} while (0)
#endif
#ifndef RAD_DEBUG_WAITLINE
#define RAD_DEBUG_WAITLINE(...) do {} while (0)
#endif

#endif  /* _RADHAL_RADBOARD_H_ */
/** @} */
