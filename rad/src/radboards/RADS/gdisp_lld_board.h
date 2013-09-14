/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#ifdef RAD_DISPLAY_ST7565
  #include "gdisp_board_st7565.h"
#else
  #error "Unsupported displays (GDISP)"
#endif

#endif /* _GDISP_LLD_BOARD_H */
/** @} */

