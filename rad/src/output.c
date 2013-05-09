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
 * @file    output.c
 * @brief   PWM Output
 *
 * @addtogroup OUTPUT
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "output.h"

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void outputInit()
{
  uint8_t i;
  RadOutputChannel *ch;
  for (i = 0; i < radboard.output.count; i++)
  {
    ch = &radboard.output.channels[i];
    pexDisableSig(ch->signal);
    palSetSigMode(ch->signal, PAL_MODE_OUTPUT_PUSHPULL);
  }
}

/**
 * @brief Set the duty cycle of an PWM output channel.
 * @param[in] ch        pointer to a @p PWMRadOutputChannel object
 * @param[in] duty      Duty cycle (0 = off, 255 = on)
 */
void outputSet(RadOutputChannel *ch, uint8_t duty)
{
  if (duty > 0)
    pwmEnableChannel(ch->pwm, ch->channel, PWM_FRACTION_TO_WIDTH(ch->pwm, 255, duty));
  else
    pwmDisableChannel(ch->pwm, ch->channel);
}

/** @} */
