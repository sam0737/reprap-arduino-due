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
 * @file    beeper.c
 * @brief   Beeper
 *
 * @addtogroup BEEPER
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "beeper.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

BeeperTune tuneStartup = { .notes = (BeeperNote[]) {
  {500, 120}, {0, 50}, {1000, 100}, {0, 0}
} };

volatile BeeperNote *note_playing;

static WORKING_AREA(waBeeper, 128);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static msg_t threadBeeper(void *arg) {
  (void)arg;
  chRegSetThreadName("beeper");

  while (TRUE) {
    while (note_playing == NULL || note_playing->len == 0)
      chThdSleepMilliseconds(100); // TODO: Use event instead

    if (note_playing->tone == 0) {
      chThdSleepMilliseconds(note_playing->len);
    } else {
      pwmChangePeriod(radboard.hmi.beeperPwm,
          radboard.hmi.beeperPwm->config->frequency / note_playing->tone);
      pwmEnableChannel(radboard.hmi.beeperPwm, 0, PWM_FRACTION_TO_WIDTH(radboard.hmi.beeperPwm, 2, 1));
      chThdSleepMilliseconds(note_playing->len);
      pwmDisableChannel(radboard.hmi.beeperPwm, 0);
    }
    note_playing++;
  }
  return 0;
}

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void beeperInit()
{
  if (radboard.hmi.beeperPwm == NULL)
    return;
  chThdCreateStatic(waBeeper, sizeof(waBeeper), NORMALPRIO, threadBeeper, NULL);
}

void beeperPlay(BeeperTune* tune)
{
  if (radboard.hmi.beeperPwm == NULL)
    return;
  note_playing = tune->notes;
}

/** @} */
