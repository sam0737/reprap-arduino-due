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

#define DISPLAY_MARQUEE_DELAY (MS2ST(1500 / TDISP_COLUMNS))
#define DISPLAY_DASHBOARD_TEMPS 2
#define DISPLAY_MENU_VISIBLE_MAX TDISP_ROWS

#include "display_control.h"
#include "ui/renderer/text/dashboard.h"
#include "ui/renderer/text/menu.h"

#include "ui/viewmodel/common/dashboard_viewmodel.h"
#include "ui/viewmodel/common/menu_viewmodel.h"
#include "ui/viewmodel/common/print_viewmodel.h"
#include "ui/viewmodel/common/prepare_viewmodel.h"
#include "ui/viewmodel/common/mainmenu_viewmodel.h"

static void display_ui_init(void) {
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
  display_control_init();
}
