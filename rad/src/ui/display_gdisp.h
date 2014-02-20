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

#include "gfx.h"

static font_t fontText;

#define DISPLAY_MARQUEE_DELAY (MS2ST(1500 / GDISP_SCREEN_WIDTH))
#define DISPLAY_DASHBOARD_TEMPS 4
#define DISPLAY_MENU_VISIBLE_MAX ((GDISP_WIDTH + 47) / 48)

static void display_ui_init(void) {
  // fontStatus = gdispOpenFont("DejaVuSans12");
  fontText = gdispOpenFont("DejaVuSans24");
  //font4 = gdispOpenFont("DejaVuSans10");
  gdispClear(Black);
}

static void display_lld_set_contrast(float contrast)
{
  gdispControl(GDISP_CONTROL_CONTRAST, (void*)(size_t)(contrast * 100));
}

