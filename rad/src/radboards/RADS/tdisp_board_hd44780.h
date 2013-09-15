/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/tdisp/HD44780/tdisp_lld_board_example.h
 * @brief   TDISP driver subsystem board interface for the HD44780 display
 *
 * @addtogroup TDISP
 * @{
 */

#define BUS_4BITS TRUE
#define PORT_RS   IOPORT3
#define PORT_CTRL IOPORT3
#define PORT_EN   IOPORT2
#define PIN_RS    28
#define PIN_EN    25
#define MASK_DATA (0xF<<23)

static void init_board(void) {
  // EN
  palSetPadMode(PORT_EN, PIN_EN, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(PORT_EN, PIN_EN);

  // RS
  palSetPadMode(PORT_RS, PIN_RS, PAL_MODE_OUTPUT_PUSHPULL);

  // D4-D7
  palSetGroupMode(IOPORT3, 0xF, 23, PAL_MODE_OUTPUT_PUSHPULL);

  // Contrast DAC
  palSetGroupMode(IOPORT2, (1<<15) | (1<<16), 0, PAL_MODE_INPUT_ANALOG);
  pmc_enable_peripheral_clock(ID_DACC);
  DACC->DACC_CR = DACC_CR_SWRST;
  DACC->DACC_MR =
      DACC_MR_TRGEN_DIS | DACC_MR_WORD_HALF | DACC_MR_USER_SEL_CHANNEL0 | //DACC_MR_TAG_EN |
      DACC_MR_STARTUP_1984 | DACC_MR_REFRESH(4);
  DACC->DACC_ACR = DACC_ACR_IBCTLCH0(0x02) | DACC_ACR_IBCTLCH1(0x02) | DACC_ACR_IBCTLDACCORE(0x01);
  DACC->DACC_CHER = DACC_CHER_CH0 | DACC_CHER_CH1;
}

static void _write_to_lcd(uint8_t data) {
  gfxSystemLock();
  palWriteGroup(IOPORT3, 0xF, 23,
      (data & 0x80 ? 1 : 0) | (data & 0x40 ? 2 : 0) | (data & 0x20 ? 4 : 0) | (data & 0x10 ? 8 : 0)
  );
  gfxSystemUnlock();
  gfxSleepMicroseconds(1);
  palSetPad(PORT_EN, PIN_EN);
  gfxSleepMicroseconds(1);
  palClearPad(PORT_EN, PIN_EN);
}

static void write_cmd(uint8_t data) {
  palClearPad(PORT_RS, PIN_RS);
  _write_to_lcd(data);
  _write_to_lcd(data<<4);
}

static void write_data(uint8_t data) {
  palSetPad(PORT_RS, PIN_RS);
  _write_to_lcd(data);
  _write_to_lcd(data<<4);
}

/** @} */

