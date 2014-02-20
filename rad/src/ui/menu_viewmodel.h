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

static void ui_dashboard_viewmodel(void);

static void ui_menu_viewmodel(void) {
  uiState.renderer = ui_menu_renderer;
  if (uiState.changed_parts & DASHBOARD_Reset)
  {
    FLUSH_INPUT(generic_wheel);
    FLUSH_INPUT(enter_button);
    FLUSH_INPUT(back_button);
    FLUSH_INPUT(up_button);
    FLUSH_INPUT(down_button);
    uiState.menu.last_refresh = chTimeNow();
    uiState.menu.encoder_delta = 0;
    uiState.menu.current = 0;
    uiState.menu.pos = 0;
  } else if (GET_INPUT(back_button).button.times) {
    uiState.menu.back_action(NULL);
    return;
  } else if (GET_INPUT(enter_button).button.times)
  {
    const UiMenuItem* menu = uiState.menu.get_cb(uiState.menu.current);
    if (menu && (!menu->visible_cb || menu->visible_cb()))
    {
      if (menu->action_cb) {
        menu->action_cb(menu->state);
        uiState.changed_parts = UI_PARTS_All;
      }
    }
    if (uiState.menu.close_cb != NULL)
      uiState.menu.close_cb();
    return;
  }

  RadInputValue value = GET_INPUT(generic_wheel);
  value.encoder.delta += uiState.menu.encoder_delta;
  uiState.menu.encoder_delta = value.encoder.delta % 4;

  int16_t offset =
      -GET_INPUT(up_button).button.times +
      GET_INPUT(down_button).button.times +
      value.encoder.delta / 4;

  if (offset || chTimeNow() - uiState.menu.last_refresh > MS2ST(500))
  {
    uiState.changed_parts |= MENU_Changed;
    uiState.menu.last_refresh = chTimeNow();
  }
  if (!offset)
    return;

  // Note: range check is done in rendering
  uiState.menu.current += offset;
  uiState.menu.offset = offset;

  uiState.menu.pos += offset;
  if (uiState.menu.pos < 0)
    uiState.menu.pos = 0;
  else if (uiState.menu.pos >= DISPLAY_MENU_VISIBLE_MAX)
    uiState.menu.pos = DISPLAY_MENU_VISIBLE_MAX - 1;
}
