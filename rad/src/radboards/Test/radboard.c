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
 * @file    RADS/radboard.c
 * @brief   Reprap Arduino Due Shield radboard configuration.
 *
 * @addtogroup RAD_BOARD
 * @{
 */

#include "hal.h"
#include "rad.h"

#include "httpmmap.h"

SerialConfig ser_shellcfg = {};
SerialConfig ser_commcfg = {};

HttpMmapDriver hmd;
HttpMmapConfig hm_config = { .port = 29100 };

static char control1_buffer[16];
HttpMmapObject hmo_control1 = {
    .name = "control1",
    .buffer = control1_buffer,
    .size = sizeof(control1_buffer)
};

static char control2_buffer[16];
HttpMmapObject hmo_control2 = {
    .name = "control2",
    .buffer = control2_buffer,
    .size = sizeof(control2_buffer)
};

void radboardInit(void)
{
  sdStart((SerialDriver*)radboard.debug.channel, &ser_shellcfg);
  sdStart((SerialDriver*)radboard.hmi.comm_channel, &ser_commcfg);

  httpmmapInit();
  httpmmapObjectInit(&hmd);
  httpmmapStart(&hmd, &hm_config);
  httpmmapAdd(&hmd, &hmo_control1);
  httpmmapAdd(&hmd, &hmo_control2);
  inputEnable(2);

  /**
   * Disable buffering on output for Eclipse console
   */
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

const radboard_t radboard =
{
    .init = &radboardInit,
    .power = {
    },
    .hmi = {
        .comm_channel = (BaseAsynchronousChannel*) &SD1
    },
    .input = {
        .channels = (RadInputChannel[]) {
          {
            .fetcher = inputVirtualEncoderFetcher,
            .config = {  .virtual_encoder = { .hmo = &hmo_control1 } },
            .processor = inputEncoderProcessor
          },
          { .processor = inputButtonProcessor },
          {
            .fetcher = inputVirtualEncoderFetcher,
            .config = {  .virtual_encoder = { .hmo = &hmo_control2 } },
            .processor = inputEncoderProcessor
          },
          { .processor = inputButtonProcessor },
        }
    },
    .output = {
        .channels = (RadOutputChannel[]) {
          { },
          { },
          { },
          { },
          { }
        }
    },
    .stepper = {
        .main_enable = { .pin = { IOPORT1, 16 }, .active_low = 0 },
        .channels = (RadStepperChannel[]) {
          {
            .step = { .pin = { IOPORT1, 0 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 1 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT1, 2 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 3 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT1, 4 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 5 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT1, 6 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 7 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT1, 8 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 9 }, .active_low = 0 },
          }
        }
    },
    .endstop = {
        .channels = (RadEndstopChannel[]) {
        }
    },
    .debug = {
        .channel = (BaseAsynchronousChannel*) &SD2,
        .software_reset = NULL,
        .erase = NULL
    },
};

/** @} */
