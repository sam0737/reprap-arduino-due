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

#ifdef RAD_DISPLAY_VIRTUAL_TEXT
  #include "tdisp_board_virtual_text.h"
#else
  #error "Unsupported displays (TDISP)"
#endif

static void display_lld_init(void)
{
}

#endif /* _TDISP_LLD_BOARD_H */
/** @} */

