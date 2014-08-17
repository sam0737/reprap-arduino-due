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
 * @file    input.h
 * @brief   Input management header
 *
 * @addtogroup ENDSTOP
 * @{
 */

#ifndef _RAD_INPUT_H_
#define _RAD_INPUT_H_


#include "radpex.h"
#if GINPUT_NEED_MOUSE
#include "gfx.h"
#elif HAS_HTTPMMAP
#include <httpmmap.h>
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef union {
#ifdef PAL_MODE_INPUT_PULLUP
  struct {
    signal_t pin_a;
    signal_t pin_b;
  } encoder;
  struct {
    signal_t pin;
  } button;
#endif
#ifdef GINPUT_NEED_MOUSE
  struct {
    uint16_t button_mask;
    GEventMouse event;
  } ginput;
#elif HAS_HTTPMMAP
  struct {
    HttpMmapObject* hmo;
  } virtual_encoder;
#endif
} RadInputConfig;

typedef struct {
  union {
    struct {
      int16_t delta;
      float rate;
    } encoder;
    struct {
      uint8_t times;
    } button;
  };
} RadInputValue;

typedef struct {
  uint8_t is_enabled;
  union {
    struct {
      uint8_t state;
      systime_t last_time;
      uint16_t last_value;
      uint16_t value;
    } encoder;
    struct {
      uint8_t is_down;
      uint8_t times;
    } button;
  };
} RadInputState;

typedef void (*input_fetch_t)(RadInputConfig* config, RadInputState* state);
typedef RadInputValue (*input_process_t)(RadInputState* state);

#ifdef __cplusplus
extern "C" {
#endif
  void inputInit(void);
  void inputEnable(uint8_t input_id);
  RadInputValue inputGet(uint8_t input_id);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_ENDSTOP_H_ */

/** @} */
