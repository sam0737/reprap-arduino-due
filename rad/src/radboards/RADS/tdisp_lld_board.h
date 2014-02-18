/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */


#ifndef _TDISP_LLD_BOARD_H
#define _TDISP_LLD_BOARD_H

#include "ch.h"
#include "hal.h"

#ifdef RAD_DISPLAY_HD44780
  #include "tdisp_board_hd44780.h"
#else
  #error "Unsupported displays (TDISP)"
#endif

static void display_lld_init(void)
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

#endif /* _TDISP_LLD_BOARD_H */


