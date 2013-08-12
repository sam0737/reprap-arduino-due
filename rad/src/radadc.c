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

static WORKING_AREA(waAdc, 128);

int8_t finishing_count = 0;
int8_t has_error = 0;
Thread* tp;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

void radadc_end_callback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;
  (void)buffer;
  (void)n;
  if (--finishing_count == 0) {
    chSysLockFromIsr();
    chSchReadyI(tp);
    chSysUnlockFromIsr();
  }
}

void radadc_error_callback(ADCDriver *adcp, adcerror_t err)
{
  (void)adcp;
  (void)err;
  has_error++;
  if (--finishing_count == 0) {
    chSysLockFromIsr();
    chSchReadyI(tp);
    chSysUnlockFromIsr();
  }
}

static msg_t threadAdc(void *arg) {
  (void)arg;
  uint8_t i, j, k, c;
  RadAdcChannel *ch;
  RadTemp *cht;

  tp = chThdSelf();
  chRegSetThreadName("adc");

  while (TRUE) {
    chThdSleepMilliseconds(50);

    finishing_count = radboard.adc.count;
    has_error = 0;

    chSysLock();
    for (i = 0; i < radboard.adc.count; i++) {
      ch = &radboard.adc.channels[i];
      adcStartConversionI(ch->adc, &ch->group_base, ch->samples, ch->group_base.num_channels);
    }
    chSchGoSleepS(THD_STATE_SUSPENDED);

    if (has_error || finishing_count)
      continue;

    c = 0;
    for (i = 0; i < radboard.adc.count; i++) {
      ch = &radboard.adc.channels[i];
      for (j = 0; j < ch->group_base.num_channels; j++, c++) {
        for (k = 0; k < machine.temperature.count; k++) {
          cht = &machine.temperature.devices[k];
          if (cht->adc_id == c && cht->converter) {
            cht->state.raw = ch->samples[j];
            cht->state.pv = cht->converter(ch->samples[j], ch->resolution);
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
    ch->group_base.end_cb = radadc_end_callback;
    ch->group_base.error_cb = radadc_error_callback;
  }
  chThdCreateStatic(waAdc, sizeof(waAdc), NORMALPRIO, threadAdc, NULL);
}

/** @} */
