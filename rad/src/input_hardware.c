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
 * @file    input_hardware.c
 * @brief   Input hardware
 *
 * @addtogroup INPUT_HARDWARE
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "input_hardware.h"

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/*
 * Note: OS is in locked state when these functions are invoked
 */

#ifdef PAL_MODE_INPUT_PULLUP
void inputButtonFetcher(RadInputConfig* config, RadInputState* state)
{
  if (state->is_enabled == 1) {
    // Initialization
    palSetSigMode(config->button.pin, PAL_MODE_INPUT_PULLUP);
    state->is_enabled = 2;
  }
  if (palReadSig(config->button.pin)) {
    if (!state->button.is_down) {
      state->button.is_down = 1;
      state->button.times++;
    }
  } else {
    state->button.is_down = 0;
  }
}

void inputEncoderFetcher(RadInputConfig* config, RadInputState* state)
{
  (void) config;
  if (state->is_enabled == 1) {
    state->is_enabled = 2;
    state->encoder.last_time = chTimeNow();
  }
}
#endif

#if GINPUT_NEED_MOUSE
void inputGinputFetcher(RadInputConfig* config, RadInputState* state)
{
  if (state->is_enabled == 1) {
    // Initialization
    ginputGetMouse(0);
    state->is_enabled = 2;
    state->encoder.last_time = chTimeNow();
  }

  ginputGetMouseStatus(0, &config->ginput.event);

  uint8_t is_down = (config->ginput.event.current_buttons & config->ginput.button_mask) ? 1 : 0;
  state->encoder.value = config->ginput.event.x / 2;
  if (abs((int)state->encoder.value - state->encoder.last_value) > 50)
    state->encoder.last_value = state->encoder.value;

  // Modifying the state of the next channel: button channel
  RadInputState* state_button = state + 1;
  if (is_down)
  {
    if (!state_button->button.is_down) {
      state_button->button.is_down = 1;
      state_button->button.times++;
    }
  } else {
    state_button->button.is_down = 0;
  }
}
#elif HAS_HTTPMMAP
#include <httpmmap.h>
extern HttpMmapDriver hmd;
void inputVirtualEncoderFetcher(RadInputConfig* config, RadInputState* state)
{
  if (state->is_enabled == 1) {
    // Initialization
    httpmmapAdd(&hmd, config->virtual_encoder.hmo);
    state->is_enabled = 2;
    state->encoder.last_time = chTimeNow();
  }

  state->encoder.value = *(uint16_t*)config->virtual_encoder.hmo->buffer;
  uint8_t is_down = *(uint8_t*)&config->virtual_encoder.hmo->buffer[4];

  // Modifying the state of the next channel: button channel
  RadInputState* state_button = state + 1;
  if (is_down)
  {
    if (!state_button->button.is_down) {
      state_button->button.is_down = 1;
      state_button->button.times++;
    }
  } else {
    state_button->button.is_down = 0;
  }
}
#endif

RadInputValue inputButtonProcessor(RadInputState* state)
{
  RadInputValue v;
  v.button.times = state->button.times;
  state->button.times = 0;
  return v;
}

RadInputValue inputEncoderProcessor(RadInputState* state)
{
  RadInputValue v;
  v.encoder.delta = state->encoder.value - state->encoder.last_value;
  state->encoder.last_value = state->encoder.value;
  v.encoder.rate =
      (float)v.encoder.delta /
      (chTimeNow() - state->encoder.last_time) * S2ST(1);
  state->encoder.last_time = chTimeNow();
  return v;
}

/** @} */
