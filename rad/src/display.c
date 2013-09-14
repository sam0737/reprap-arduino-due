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
 * @file    hmi.c
 * @brief   Human Machine Interface
 *
 * @addtogroup HMI
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "display.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waDisplay, 2048);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

#define HAS_DISPLAY  (GFX_USE_TDISP || GFX_USE_GDISP)

#if GFX_USE_TDISP
#endif
#if GFX_USE_GDISP
#endif
#include "hmi/display_gdisp.h"

#if HAS_DISPLAY

inline static void displayUpdate(void) {
  display_test();
}

static msg_t threadDisplay(void *arg) {
  (void)arg;
  systime_t next;
  systime_t now;

  chRegSetThreadName("display");
  display_init();

  while (TRUE) {
    next = chTimeNow() + MS2ST(33); // 30 fps
    displayUpdate();
    now = chTimeNow();
    chThdSleep(now + MS2ST(1) > next ? MS2ST(1) : next - now);

  }
  return 0;
}
#endif

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void displayInit(void)
{
#if HAS_DISPLAY
  gfxInit();
  chThdCreateStatic(waDisplay, sizeof(waDisplay), NORMALPRIO - 2, threadDisplay, NULL);
#endif
}

/** @} */
