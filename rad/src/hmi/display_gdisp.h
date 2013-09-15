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
 * @file    display_gdisp.h
 * @brief   GDISP Human Machine Interface
 *
 * @addtogroup DISPLAY_GDISP
 * @{
 */

#include "gfx.h"

font_t font1;
font_t font2;
font_t font3;
font_t font4;

static void display_init(void) {
  font1 = gdispOpenFont("DejaVuSans12");
  font2 = gdispOpenFont("DejaVuSans12");
  font3 = gdispOpenFont("UbuntuMono12");
  font4 = gdispOpenFont("UbuntuMono12");
  //font4 = gdispOpenFont("DejaVuSans10");
  gdispClear(Black);
}

static void display_test(void) {
  char buf[20];
  snprintf(buf, 20, "cfp.Tesg %d", (int) chTimeNow());
  gdispFillStringBox(0,0,128,16, buf,font1,Black,White,justifyCenter);
  gdispFillStringBox(0,16,128,16,buf,font2,White,Black,justifyCenter);
  gdispFillStringBox(0,32,128,16,buf,font3,Black,White,justifyCenter);
  gdispFillStringBox(0,48,128,16,buf,font4,White,Black,justifyCenter);
  gdispControl(GDISP_CONTROL_LLD_FLUSH, NULL);
}

static void display_lld_set_contrast(float contrast)
{
  gdispControl(GDISP_CONTROL_CONTRAST, (void*)(size_t)(contrast * 100));
}

/** @} */
