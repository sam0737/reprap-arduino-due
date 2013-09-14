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

#ifndef _TDISP_LLD_BOARD_H
#define _TDISP_LLD_BOARD_H

#ifdef RAD_DISPLAY_HD44780
  #include "tdisp_board_hd44780.h"
#else
  #error "Unsupported displays (TDISP)"
#endif

#endif /* _TDISP_LLD_BOARD_H */
/** @} */

