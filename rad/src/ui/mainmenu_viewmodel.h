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

static uint8_t ui_mainmenu_can_estop_clear(void) {
  return printerIsEstopped() != 0;
}
static void ui_mainmenu_do_estop_clear(void* state) {
  printerEstopClear();
}

static uint8_t ui_mainmenu_can_resume(void) {
  return printerGetState() == PRINTERSTATE_Interrupted;
}
static void ui_mainmenu_do_resume_now(void* state) {
  printerResume(PRINTINGSOURCE_Lcd);
}

static uint8_t ui_mainmenu_can_tuning(void) {
  return printerGetState() & PRINTERSTATE_Printing;
}

static uint8_t ui_mainmenu_can_interrupt_now(void) {
  return printerGetState() & PRINTERSTATE_Printing;
}
static void ui_mainmenu_do_interrupt_now(void* state) {
  printerInterrupt(PRINTINGSOURCE_Lcd);
}

static uint8_t ui_mainmenu_can_print(void) {
  return printerGetState() == PRINTERSTATE_Standby;
}
static uint8_t ui_mainmenu_can_storage_ums(void) {
  return storageGetHostState() == STORAGE_Local; /** TODO: Not printing by SD **/
}
static void ui_mainmenu_do_storage_ums(void* state) {
  storageUsbMount();
}
static uint8_t ui_mainmenu_can_storage_local(void) {
  return storageGetHostState() == STORAGE_Usb;
}
static void ui_mainmenu_do_storage_local(void* state) {
  storageUsbUnmount();
}
static uint8_t ui_mainmenu_can_prepare(void) {
  return printerGetState() == PRINTERSTATE_Interrupted
      || printerGetState() == PRINTERSTATE_Standby;
}
static uint8_t ui_mainmenu_can_estop(void) {
  return !printerIsEstopped();
}
static void ui_mainmenu_do_estop(void* state) {
  printerEstop (L_DEBUG_STOPPED_BY_MENU);
}
static uint8_t ui_mainmenu_can_power_off(void) {
  return powerCanControlPsu()
      && (printerGetState() == PRINTERSTATE_Standby
          || printerGetState() == PRINTERSTATE_Estopped);
}
static void ui_mainmenu_do_power_off(void* state) {
  powerPsuOff();
}

static const UiStandardMenu ui_mainmenu =
{
  .count = 12,
  .menus = (UiMenuItem[]) {
    {
      .name = L_UI_BACK,
      .suffix = '^',
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
      .visible_cb = ui_mainmenu_can_resume,
      .action_cb = ui_mainmenu_do_resume_now
    },
    {
      .name = L_UI_MAINMENU_TUNING,
      .suffix = '>',
      .visible_cb = ui_mainmenu_can_tuning,
    },
    {
      .name = L_UI_MAINMENU_INTERRUPT_NOW,
      .visible_cb = ui_mainmenu_can_interrupt_now,
      .action_cb = ui_mainmenu_do_interrupt_now
    },
    {
      .name = L_UI_MAINMENU_PRINT,
      .suffix = '>',
      .visible_cb = ui_mainmenu_can_print,
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
      .suffix = '>',
      .visible_cb = ui_mainmenu_can_prepare,
      .action_cb = ui_menu_goto_page,
      .state = ui_prepare_viewmodel,
    },
    {
      .name = L_UI_MAINMENU_INFO,
      .suffix = '>',
    },
    {
      .name = L_UI_MAINMENU_ESTOP,
      .visible_cb = ui_mainmenu_can_estop,
      .action_cb = ui_mainmenu_do_estop
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
  uiState.menu.get_next_cb = uiStandardMenuGetNext;
  uiState.menu.close_cb = NULL;
  uiState.menu.back_cb = ui_menu_goto_dashboard;
  uiState.menu.standard.menu = &ui_mainmenu;
}
