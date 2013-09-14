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
#include "usbcfg.h"

/**
 * Emulate Arduino Due erase the chip upon being connected at 1200bps
 */
void arduino_erase_condition_callback(SerialUSBDriver *sdup)
{
  if (*((uint32_t*)&sdup->line_coding) == 1200) {
    if (!(sdup->control_line_state & USB_SERIAL_DTR)) {
      debugErase();
    }
  }
}

/**
 * Actual erase code
 */
void debug_erase(void)
{
  chSysLock();

  // Set bootflag to run SAM-BA bootloader at restart
  const int EEFC_FCMD_CGPB = 0x0C;
  const int EEFC_KEY = 0x5A;
  while (!(EFC0->EEFC_FSR & EEFC_FSR_FRDY));
  EFC0->EEFC_FCR =
    EEFC_FCR_FCMD(EEFC_FCMD_CGPB) |
    EEFC_FCR_FARG(1) |
    EEFC_FCR_FKEY(EEFC_KEY);
  while (!(EFC0->EEFC_FSR & EEFC_FSR_FRDY));

  // Reset
  const int RSTC_KEY = 0xA5;
  RSTC->RSTC_CR = RSTC_CR_KEY(RSTC_KEY) | RSTC_CR_PROCRST | RSTC_CR_PERRST;

  while (1);
}

void debug_software_reset(void)
{
  RSTC->RSTC_CR = RSTC_CR_KEY(0xA5) | RSTC_CR_PROCRST | RSTC_CR_PERRST;
  while (1);
}

static void hmi_tdisp_set_contrast_init(void)
{
  palSetGroupMode(IOPORT2, (1<<15) | (1<<16), 0, PAL_MODE_INPUT_ANALOG);
  pmc_enable_peripheral_clock(ID_DACC);
  DACC->DACC_CR = DACC_CR_SWRST;
  DACC->DACC_MR =
      DACC_MR_TRGEN_DIS | DACC_MR_WORD_HALF | DACC_MR_USER_SEL_CHANNEL0 | //DACC_MR_TAG_EN |
      DACC_MR_STARTUP_1984 | DACC_MR_REFRESH(4);
  DACC->DACC_ACR = DACC_ACR_IBCTLCH0(0x02) | DACC_ACR_IBCTLCH1(0x02) | DACC_ACR_IBCTLDACCORE(0x01);
  DACC->DACC_CHER = DACC_CHER_CH0 | DACC_CHER_CH1;
}

static void hmi_set_tdisp_contrast(float contrast)
{
  (void) contrast;
  DACC->DACC_CDR =
      0xFFF &
      ((uint32_t) (contrast > 1 ? 1 : contrast < 0 ? 0 : contrast * 0xFFF));
}

static const PWMConfig beeper_cfg = {
  .frequency = SYSTEM_CLOCK / 128,
  .period = (SYSTEM_CLOCK / 128) / 1000,
  .channels = { { .h_pin = {PIOC, 18, PIO_MODE_B} } }
};

static const PWMConfig output_cfg[] = { {
  .frequency = SYSTEM_CLOCK / 1024, .period = 255 * 2,
  .channels = { { .mode = PWM_CHANNEL_POLARITY_HIGH, .h_pin = {IOPORT3, 3, PIO_MODE_B} } }
},
{
  .frequency = SYSTEM_CLOCK / 1024, .period = 255 * 2,
  .channels = { { .mode = PWM_CHANNEL_POLARITY_HIGH, .h_pin = {IOPORT3, 5, PIO_MODE_B} } }
},
{
  .frequency = SYSTEM_CLOCK / 1024, .period = 255 * 2,
  .channels = { { .mode = PWM_CHANNEL_POLARITY_HIGH, .h_pin = {IOPORT3, 7, PIO_MODE_B} } }
},
{
  .frequency = SYSTEM_CLOCK / 1024, .period = 255 * 2,
  .channels = { { .mode = PWM_CHANNEL_POLARITY_HIGH, .h_pin = {IOPORT3, 9, PIO_MODE_B} } }
},
{
  .frequency = SYSTEM_CLOCK / 1024, .period = 255 * 2,
  .channels = { { .mode = PWM_CHANNEL_POLARITY_HIGH, .h_pin = {IOPORT3, 19, PIO_MODE_B} } }
} };

static ADCConfig adc_cfg = {
    .frequency = 1 * 1000 * 1000,
    .use_sequence = 1,
    // We want the ADC captures in the sequence of CH7,CH6,CH5,CH15
    // The USCH number is 1-based, while the CH is 0-based.
    .sequence1 = ADC_SEQR1_USCH6(7) | // PA23/E1
                 ADC_SEQR1_USCH7(6) | // PA24/E0
                 ADC_SEQR1_USCH8(5),  // PA16/HBP
    .sequence2 = ADC_SEQR2_USCH16(15)
};

static GPTConfig gpt_stepper_cfg = {
    .frequency = SYSTEM_CLOCK / 2
};

static SPIConfig spi_lscfg = {
    .spi_mode = 0,
    .speed = 300000,
    .spck_pin = { IOPORT1, 27, PIO_MODE_A },
    .miso_pin = { IOPORT1, 25, PIO_MODE_A },
    .mosi_pin = { IOPORT1, 26, PIO_MODE_A },
    .cs_pin = { IOPORT2, 14, 0 },
    .use_dma = TRUE,
    .dma_tx_ch = 0,
    .dma_rx_ch = 1,
};

static SPIConfig spi_hscfg = {
    .spi_mode = 0,
    .speed = 20000000,
    .spck_pin = { IOPORT1, 27, PIO_MODE_A },
    .miso_pin = { IOPORT1, 25, PIO_MODE_A },
    .mosi_pin = { IOPORT1, 26, PIO_MODE_A },
    .cs_pin = { IOPORT2, 14, 0 },
    .use_dma = TRUE,
    .dma_tx_ch = 0,
    .dma_rx_ch = 1,
};

static MMCConfig mmc_cfg = {
    .spip = &SPID1,
    .lscfg = &spi_lscfg,
    .hscfg = &spi_hscfg
};

MMCDriver MMCD1;

void radboardInit(void)
{
  uint8_t i;
  // Disable Watchdog
  WDT->WDT_MR = WDT_MR_WDDIS;

  pwmStart(radboard.hmi.beeper_pwm, &beeper_cfg);

  for (i = 0; i < radboard.output.count; i++) {
    pwmStart(radboard.output.channels[i].pwm, &output_cfg[i]);
  }

  adcStart(&ADCD1, &adc_cfg);

  serusb_shellcfg.controllinestate_cb = &arduino_erase_condition_callback;
  sduObjectInit((SerialUSBDriver*) radboard.debug.channel);
  sduStart((SerialUSBDriver*)radboard.debug.channel, &serusb_shellcfg);

  sduObjectInit(&SDU_DATA);
  sduStart(&SDU_DATA, &serusb_datacfg);

  mmcObjectInit(&MMCD1);
  mmcStart(&MMCD1, &mmc_cfg);

  msdObjectInit(&UMSD);
  msdStart(&UMSD, &ums_cfg);

  hmi_tdisp_set_contrast_init();
  hmi_set_tdisp_contrast(0.5);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(&USBD1);
  usbStart(&USBD1, &usbcfg);
  chThdSleepMilliseconds(500);
  usbConnectBus(&USBD1);
}

const radboard_t radboard =
{
    .init = &radboardInit,
    .power = {
        .psu_on = { .pin = { .port = IOPORT2, .pin = 20 }, .active_low = 1 }
    },
    .hmi = {
        .beeper_pwm = &PWMD7,
        .beeper_channel = 0,
        .set_tdisp_contrast = hmi_set_tdisp_contrast,
        .comm_channel = (BaseAsynchronousChannel*) &SDU_DATA,
        .storage_device = (BaseBlockDevice*) &MMCD1,
        .usb_msd = &UMSD,
    },
    .output = {
        .count = 5,
        .channels = (RadOutputChannel[]) {
          { .pwm = &PWMD1, .channel = 0,
            .signal = { .pin = { IOPORT3, 3 }, .active_low = 0 } },
          { .pwm = &PWMD2, .channel = 0,
            .signal = { .pin = { IOPORT3, 5 }, .active_low = 0 } },
          { .pwm = &PWMD3, .channel = 0,
            .signal = { .pin = { IOPORT3, 7 }, .active_low = 0 } },
          { .pwm = &PWMD4, .channel = 0,
            .signal = { .pin = { IOPORT3, 9 }, .active_low = 0 } },
          { .pwm = &PWMD6, .channel = 0,
            .signal = { .pin = { IOPORT3, 19 }, .active_low = 0 } }
        }
    },
    .stepper = {
        .count = RAD_NUMBER_STEPPERS,
        .gpt = &GPTD1,
        .gpt_config = &gpt_stepper_cfg,
        .main_enable = { .pin = { IOPORT3, 2 }, .active_low = 1 },
        .channels = (RadStepperChannel[]) {
          {
            .step = { .pin = { IOPORT2, 26 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 14 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT1, 15 }, .active_low = 0 },
            .dir = { .pin = { IOPORT4, 0 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT4, 1 }, .active_low = 0 },
            .dir = { .pin = { IOPORT4, 2 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT4, 3 }, .active_low = 0 },
            .dir = { .pin = { IOPORT4, 6 }, .active_low = 0 },
          },
          {
            .step = { .pin = { IOPORT4, 9 }, .active_low = 0 },
            .dir = { .pin = { IOPORT1, 7 }, .active_low = 0 },
          }
        }
    },
    .endstop = {
        .count = 3,
        .channels = (RadEndstopChannel[]) {
          { .pin = { IOPORT3, 4 } },
          { .pin = { IOPORT3, 6 } },
          { .pin = { IOPORT3, 8 } }
        }
    },
    .adc = {
        .count = 1,
        .channels = (RadAdcChannel[]) {
          {
            .adc = &ADCD1,
            .resolution = 12,
            .group_base = {
                .num_channels = 4,
                .channel_mask = 0x80E0, // Ch 5,6,7,15
            },
            .samples = (adcsample_t[]) { 0, 0, 0, 0 }
          }
        }
    },
    .debug = {
        .heartbeat_led= { .port = IOPORT2, .pin = 27 },
        .channel = (BaseAsynchronousChannel*) &SDU_SHELL,
        .software_reset = &debug_software_reset,
        .erase = &debug_erase
    }
};

/**
 * @brief   MMC_SPI card detection.
 */
bool_t mmc_lld_is_card_inserted(MMCDriver *mmcp) {
  if (mmcp->state == BLK_STOP || mmcp->state == BLK_UNINIT)
    return FALSE;

  if (blkIsTransferring(mmcp))
    return TRUE;

  if (mmcp->state == BLK_READY &&
      blkRead(mmcp, 0, NULL, 0) == CH_SUCCESS)
    return TRUE;

  return blkConnect(mmcp) == CH_SUCCESS;
}

/**
 * @brief   MMC_SPI card write protection detection.
 */
bool_t mmc_lld_is_write_protected(MMCDriver *mmcp) {

  (void)mmcp;
  /* TODO: Fill the implementation.*/
  return FALSE;
}
/** @} */
