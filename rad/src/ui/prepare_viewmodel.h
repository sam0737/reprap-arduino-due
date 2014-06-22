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

static void ui_prepare_back(void)
{
  printerRelease(PRINTINGSOURCE_Lcd);
  uiChangePage(ui_mainmenu_viewmodel);
}

static void ui_prepare_back_action(void* state)
{
  (void) state;
  ui_prepare_back();
}

static void ui_prepare_do_homing(void* state)
{
  PrinterCommand cmd;
  gcodeInitializeCommand(&cmd);
  cmd.code = 28;
  printerPushCommand(PRINTINGSOURCE_Lcd, &cmd);
}

static const UiStandardMenu ui_prepare =
{
    .count = 2,
    .menus = (UiMenuItem[]) {
      {
          .name = L_UI_BACK,
          .suffix = '^',
          .visible_cb = ui_menu_shows_back_item,
          .action_cb = ui_prepare_back_action
      },
      {
          .name = L_UI_PREPARE_HOMING,
          .action_cb = ui_prepare_do_homing
      }
    }
};

static void ui_prepare_viewmodel(void) {
  uiState.viewmodel = ui_menu_viewmodel;
  if (!printerTryAcquire(PRINTINGSOURCE_Lcd))
  {
    uiChangePage(ui_mainmenu_viewmodel);
    return;
  }
  uiState.menu.get_cb = uiStandardMenuGet;
  uiState.menu.count_cb = uiStandardMenuCount;
  uiState.menu.get_next_cb = uiStandardMenuGetNext;
  uiState.menu.close_cb = NULL;
  uiState.menu.back_cb = ui_prepare_back;
  uiState.menu.standard.menu = &ui_prepare;
}
