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

SerialConfig ser_shellcfg = {};
SerialConfig ser_commcfg = {};

void radboardInit(void)
{
  sdStart((SerialDriver*)radboard.debug.channel, &ser_shellcfg);
  sdStart((SerialDriver*)radboard.hmi.comm_channel, &ser_commcfg);

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
    .endstop = {
        .count = 0,
        .channels = (RadEndstopChannel[]) {
        }
    },
    .debug = {
        .channel = (BaseAsynchronousChannel*) &SD2,
        .software_reset = NULL,
        .erase = NULL
    }
};

/** @} */