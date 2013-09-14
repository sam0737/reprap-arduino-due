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
#include "usb_msd.h"
#include "chevents.h"

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
  Pio         *port;

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
  PWMDriver       *pwm;
  pwmchannel_t    channel;
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

typedef struct {
  ADCDriver       *adc;
  uint8_t         resolution;
  ADCConversionGroup  group_base;
  adcsample_t     *samples;
} RadAdcChannel;

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
    PWMDriver       *beeper_pwm;
    pwmchannel_t    beeper_channel;

    /**
     * @brief Handler for setting Text LCD contrast
     */
    void (*set_tdisp_contrast)(float contrast);

    /**
     * @brief Host communication channel
     */
    BaseAsynchronousChannel  *comm_channel;

    BaseBlockDevice          *storage_device;
    USBMassStorageDriver     *usb_msd;
  } hmi;

  /*******************************************
   * @brief PWM Output
   */
  struct {
    uint8_t                   count;
    RadOutputChannel          *channels;
  } output;

  /*******************************************
   * @brief Stepper Output
   */
  struct {
    uint8_t                   count;
    signal_t                  main_enable;
    GPTDriver                 *gpt;
    GPTConfig                 *gpt_config;
    RadStepperChannel         *channels;
  } stepper;

  /*******************************************
   * @brief Endstop Input
   */
  struct {
    uint8_t                   count;
    RadEndstopChannel         *channels;
  } endstop;

  /*******************************************
   * @brief ADC Input
   */
  struct {
    uint8_t                   count;
    RadAdcChannel             *channels;
  } adc;

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

#include "radboard.h"

#ifndef SYSTEM_CLOCK
#error "Please define SYSTEM_CLOCK in radboard.h - System clock in Hz"
#endif

#ifndef RAD_NUMBER_STEPPERS
#error "Please define RAD_NUMBER_STEPPERS in radboard.h"
#endif

#endif  /* _RADHAL_RADBOARD_H_ */
/** @} */
