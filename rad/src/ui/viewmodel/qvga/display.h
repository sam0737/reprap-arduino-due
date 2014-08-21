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

font_t fontTitle;
font_t fontText;
font_t fontSmall;

#define DISPLAY_MARQUEE_DELAY (MS2ST(1500 / GDISP_SCREEN_WIDTH))
#define DISPLAY_DASHBOARD_TEMPS   4
#define DISPLAY_MENU_LINE_HEIGHT  32
#define DISPLAY_MENU_ICON_WIDTH   64

#define DISPLAY_WIDTH       (GDISP_SCREEN_HEIGHT > GDISP_SCREEN_WIDTH ? GDISP_SCREEN_HEIGHT : GDISP_SCREEN_WIDTH)
#define DISPLAY_HEIGHT      (GDISP_SCREEN_HEIGHT > GDISP_SCREEN_WIDTH ? GDISP_SCREEN_WIDTH : GDISP_SCREEN_HEIGHT)

#ifdef DISPLAY_MENU_LINE_HEIGHT
  #define DISPLAY_MENU_VISIBLE_LINE_MAX ((GDISP_SCREEN_HEIGHT) / (DISPLAY_MENU_LINE_HEIGHT))
#else
  #error "DISPLAY_MENU_LINE_HEIGHT must be defined in gfxconf.h"
#endif
#define DISPLAY_MENU_VISIBLE_MAX DISPLAY_MENU_VISIBLE_LINE_MAX

#ifdef DISPLAY_MENU_ICON_WIDTH
  #define DISPLAY_MENU_VISIBLE_ICON_MAX ((GDISP_SCREEN_WIDTH) / (DISPLAY_MENU_ICON_WIDTH))
#else
  #error "DISPLAY_MENU_ICON_WIDTH must be defined in gfxconf.h"
#endif

#include "display_control.h"
#include "ui/renderer/qvga/color.h"
#include "ui/renderer/qvga/dashboard.h"
#include "ui/renderer/qvga/menu.h"

#include "ui/viewmodel/common/dashboard_viewmodel.h"
#include "ui/viewmodel/common/menu_viewmodel.h"
#include "ui/viewmodel/common/print_viewmodel.h"
#include "ui/viewmodel/common/prepare_viewmodel.h"
#include "ui/viewmodel/common/mainmenu_viewmodel.h"

static void display_ui_init(void) {
  gdispSetBacklight(100);
#if GDISP_SCREEN_HEIGHT > GDISP_SCREEN_WIDTH
  gdispSetOrientation(GDISP_ROTATE_90);
#endif

  fontTitle = gdispOpenFont("DejaVuSans32");
  fontText = gdispOpenFont("DejaVuSans24");
  fontSmall = gdispOpenFont("DejaVuSans16");

  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    switch (machine.kinematics.axes[i].name)
    {
    case AXIS_X:
      axis_ids[0] = i;
      break;
    case AXIS_Y:
      axis_ids[1] = i;
      break;
    case AXIS_Z:
      axis_ids[2] = i;
      break;
    default:
      break;
    }
  }

  int8_t always_homing_joint_id = -1;
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++)
  {

    if (machine.kinematics.joints[i].home_sequence < 0)
    {
      always_homing_joint_id = i;
      break;
    }
    switch (machine.kinematics.joints[i].home_axis_name)
    {
    case AXIS_X:
      joint_ids[0] = i;
      break;
    case AXIS_Y:
      joint_ids[1] = i;
      break;
    case AXIS_Z:
      joint_ids[2] = i;
      break;
    default:
      break;
    }
  }
  if (always_homing_joint_id >= 0) {
    joint_ids[0] = always_homing_joint_id;
    joint_ids[1] = always_homing_joint_id;
    joint_ids[2] = always_homing_joint_id;
  }
  display_control_init();
}

