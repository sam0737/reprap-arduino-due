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

extern const BeeperTune tuneStartup, tuneFinished, tuneOk, tuneWarning;

const BeeperTune tuneStartup = { .notes = (BeeperNote[]) {
  {500, 60}, {0, 50}, {1000, 50}, {0, 0}
} };

const BeeperTune tuneFinished = { .notes = (BeeperNote[]) {
  {523, 150}, {0, 20}, {520, 150}, {0, 20}, {659, 150}, {0, 10}, {1046, 100}, {0, 10}, {0, 0}
} };

const BeeperTune tuneOk = { .notes = (BeeperNote[]) {
  {440, 80}, {0, 10}, {0, 0}
} };

const BeeperTune tuneScroll = { .notes = (BeeperNote[]) {
  {50, 10}, {0, 0}
} };

const BeeperTune tuneWarning = { .notes = (BeeperNote[]) {
  {1000, 150}, {0, 10}, {1000, 80}, {0, 10}, {0, 0}
} };


#if HAL_USE_PWM
static BeeperNote* volatile note_playing;
static BinarySemaphore bsemNewNote;

static WORKING_AREA(waBeeper, 128);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static msg_t threadBeeper(void *arg) {
  (void)arg;
  BeeperNote* note;
  chRegSetThreadName("beeper");

  while (TRUE) {
    while (note_playing == NULL || note_playing->len == 0)
      chBSemWait(&bsemNewNote);

    note = note_playing++;
    if (note->tone == 0) {
      chThdSleepMilliseconds(note->len);
    } else {
      pwmChangePeriod(radboard.hmi.beeper_pwm,
          radboard.hmi.beeper_pwm->config->frequency / note->tone);
      pwmEnableChannel(radboard.hmi.beeper_pwm, 0, PWM_FRACTION_TO_WIDTH(radboard.hmi.beeper_pwm, 2, 1));
      chThdSleepMilliseconds(note->len);
      pwmDisableChannel(radboard.hmi.beeper_pwm, 0);
    }
  }
  return 0;
}

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void beeperInit()
{
  if (radboard.hmi.beeper_pwm == NULL)
    return;
  chBSemInit(&bsemNewNote, TRUE);
  chThdCreateStatic(waBeeper, sizeof(waBeeper), NORMALPRIO, threadBeeper, NULL);
}

void beeperPlay(const BeeperTune* tune)
{
  if (radboard.hmi.beeper_pwm == NULL)
    return;
  note_playing = tune->notes;
  chBSemSignal(&bsemNewNote);
}

#else
void beeperInit(){}
void beeperPlay(const BeeperTune* tune){}
#endif

/** @} */
