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

#if HAL_USE_PWM
/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

static uint8_t outputs[RAD_NUMBER_OUTPUTS];

void outputInit()
{
  uint8_t i;
  RadOutputChannel *ch;
  for (i = 0; i < RAD_NUMBER_OUTPUTS; i++)
  {
    ch = &radboard.output.channels[i];
    pexDisableSig(ch->signal);
    palSetSigMode(ch->signal, PAL_MODE_OUTPUT_PUSHPULL);
  }
}

static void outputSetI(uint8_t output_id, uint8_t duty)
{
  RadOutputChannel *ch = &radboard.output.channels[output_id];
  if (duty > 0 && !printerIsEstopped())
    pwmEnableChannel(ch->pwm, ch->channel, PWM_FRACTION_TO_WIDTH(ch->pwm, 255, duty));
  else
    pwmDisableChannel(ch->pwm, ch->channel);
  outputs[output_id] = duty;
}

/**
 * @brief Set the duty cycle of an PWM output channel.
 * @param[in] ch        PWM output channel number
 * @param[in] duty      Duty cycle (0 = off, 255 = on)
 */
void outputSet(uint8_t output_id, uint8_t duty)
{
  if (output_id > RAD_NUMBER_OUTPUTS)
    return;

  chSysLock();
  outputSetI(output_id, duty);
  chSysUnlock();
}

uint8_t outputGet(uint8_t output_id)
{
  uint8_t duty;

  if (output_id > RAD_NUMBER_OUTPUTS)
    return 0;
  chSysLock();
  duty = outputs[output_id];
  chSysUnlock();
  return duty;
}

void outputAllZero(void)
{
  chSysLock();
  for (uint8_t i = 0; i < RAD_NUMBER_OUTPUTS; i++)
    outputSetI(i, 0);
  chSysUnlock();
}

#else

static uint8_t outputs[RAD_NUMBER_OUTPUTS];

void outputInit(void)
{

}

void outputSet(uint8_t output_id, uint8_t duty)
{
  if (output_id >= RAD_NUMBER_OUTPUTS)
    return;
  chSysLock();
  outputs[output_id] = printerIsEstopped() ? 0 : duty;
  chSysUnlock();
}

uint8_t outputGet(uint8_t output_id)
{
  uint8_t duty;

  if (output_id >= RAD_NUMBER_OUTPUTS)
    return 0;
  chSysLock();
  duty = outputs[output_id];
  chSysUnlock();
  return duty;
}

void outputAllZero(void)
{
  for (uint8_t i = 0; i < RAD_NUMBER_OUTPUTS; i++)
    outputSet(i, 0);
}

#endif

/** @} */
