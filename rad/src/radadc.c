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
 * @file    radadc.c
 * @brief   RAD ADC
 *
 * @addtogroup RADADC
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "radadc.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waAdc, 256);

volatile int8_t finishingCount = 0;
volatile int8_t hasError = 0;
Thread* tp;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

void radadcEndCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;
  (void)buffer;
  (void)n;
  if (--finishingCount == 0) {
    chSysLockFromIsr();
    chSchReadyI(tp);
    chSysUnlockFromIsr();
  }
}

void radadcErrorCallback(ADCDriver *adcp, adcerror_t err)
{
  (void)adcp;
  (void)err;
  hasError++;
  if (--finishingCount == 0) {
    chSysLockFromIsr();
    chSchReadyI(tp);
    chSysUnlockFromIsr();
  }
}

static msg_t threadAdc(void *arg) {
  (void)arg;
  uint8_t i, j, k, c;
  RadAdcChannel *ch;
  RadTempChannel *cht;

  tp = chThdSelf();
  chRegSetThreadName("adc");

  while (TRUE) {
    chThdSleepMilliseconds(50);

    finishingCount = radboard.adc.count;
    hasError = 0;

    chSysLock();
    for (i = 0; i < radboard.adc.count; i++) {
      ch = &radboard.adc.channels[i];
      adcStartConversionI(ch->adc, &ch->group_base, ch->samples, ch->group_base.num_channels);
    }
    chSchGoSleepS(THD_STATE_SUSPENDED);

    if (hasError || finishingCount)
      continue;

    c = 1;
    for (i = 0; i < radboard.adc.count; i++) {
      ch = &radboard.adc.channels[i];
      for (j = 0; j < ch->group_base.num_channels; j++, c++) {
        for (k = 0; k < machine.extruders.count; k++) {
          cht = &machine.extruders.channels[k];
          if (cht->adcId == c && cht->converter) {
            cht->pv = cht->converter(ch->samples[j], ch->resolution);
          }
        }
        for (k = 0; k < machine.heatedBeds.count; k++) {
          cht = &machine.heatedBeds.channels[k];
          if (cht->adcId == c && cht->converter) {
            cht->pv = cht->converter(ch->samples[j], ch->resolution);
          }
        }
        for (k = 0; k < machine.tempMonitors.count; k++) {
          cht = &machine.tempMonitors.channels[k];
          if (cht->adcId == c && cht->converter) {
            cht->pv = cht->converter(ch->samples[j], ch->resolution);
          }
        }
      }
    }
  }
  return 0;
}

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void radadcInit()
{
  uint8_t i;
  RadAdcChannel *ch;

  for (i = 0; i < radboard.adc.count; i++) {
    ch = &radboard.adc.channels[i];
    ch->group_base.end_cb = radadcEndCallback;
    ch->group_base.error_cb = radadcErrorCallback;
  }
  chThdCreateStatic(waAdc, sizeof(waAdc), NORMALPRIO, threadAdc, NULL);
}

/** @} */
