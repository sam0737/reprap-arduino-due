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

static uint8_t ui_mainmenu_can_estop_clear(void) { return printerIsEstopped(); }
static void ui_mainmenu_do_estop_clear(void* state) { printerEstopClear(); }
static uint8_t ui_mainmenu_can_power_off(void) { return powerCanControlPsu(); }
static void ui_mainmenu_do_power_off(void* state) { powerPsuOff(); }
static uint8_t ui_mainmenu_can_storage_ums(void) { return storageGetHostState() == STORAGE_Local; }
static void ui_mainmenu_do_storage_ums(void* state) { storageUsbMount(); }
static uint8_t ui_mainmenu_can_storage_local(void) { return storageGetHostState() == STORAGE_Usb; }
static void ui_mainmenu_do_storage_local(void* state) { storageUsbUnmount(); }

static const UiStandardMenu ui_mainmenu =
{
    .count = 11,
    .menus = (UiMenuItem[]) {
      {
          .name = L_UI_BACK,
          .visible_cb = ui_menu_shows_back_item,
          .action_cb = ui_menu_goto_page,
          .state = ui_dashboard_viewmodel
      },
      {
          .name = L_UI_MAINMENU_ESTOP_CLEAR,
          .visible_cb = ui_mainmenu_can_estop_clear,
          .action_cb = ui_mainmenu_do_estop_clear
      },
      {
          .name = L_UI_MAINMENU_RESUME_PRINTING,
      },
      {
          .name = L_UI_MAINMENU_TUNING,
      },
      {
          .name = L_UI_MAINMENU_INTERRUPT_NOW,
      },
      {
          .name = L_UI_MAINMENU_PRINT,
          .action_cb = ui_menu_goto_page,
          .state = ui_print_viewmodel
      },
      {
          .name = L_UI_MAINMENU_STORAGE_UMS,
          .visible_cb = ui_mainmenu_can_storage_ums,
          .action_cb = ui_mainmenu_do_storage_ums
      },
      {
          .name = L_UI_MAINMENU_STORAGE_LOCAL,
          .visible_cb = ui_mainmenu_can_storage_local,
          .action_cb = ui_mainmenu_do_storage_local
      },
      {
          .name = L_UI_MAINMENU_PREPARE,
      },
      {
          .name = L_UI_MAINMENU_INFO,
      },
      {
          .name = L_UI_MAINMENU_POWER_OFF,
          .visible_cb = ui_mainmenu_can_power_off,
          .action_cb = ui_mainmenu_do_power_off
      }
    }
};

static void ui_mainmenu_viewmodel(void) {
  uiState.viewmodel = ui_menu_viewmodel;
  uiState.menu.get_cb = uiStandardMenuGet;
  uiState.menu.count_cb = uiStandardMenuCount;
  uiState.menu.get_next_cb = NULL;
  uiState.menu.close_cb = NULL;
  uiState.menu.state = &ui_mainmenu;
  uiState.menu.back_action = ui_menu_goto_dashboard;
}
