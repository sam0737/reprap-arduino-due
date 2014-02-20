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

static void ui_menu_renderer(void) {
  if (uiState.changed_parts & DASHBOARD_Reset)
  {
    tdispClear();
  }

  if (uiState.changed_parts & MENU_Changed)
  {
    for (uint8_t i = 0; i < TDISP_ROWS; i++)
    {
      tdispSetCursor(0, i);
      const UiMenuItem* item =
          uiState.menu.get_next_cb != NULL && i > 0 ?
          uiState.menu.get_next_cb() :
          uiMenuGetItem(i);
      tdispDrawChar(i == uiState.menu.pos ? '>' : ' ');
      if (item != NULL) {
        tdispDrawString(item->name);
        for (uint8_t j = TDISP_COLUMNS - strlen(item->name) - 1; j > 0; j--)
          tdispDrawChar(' ');
      }
    }
    if (uiState.menu.close_cb != NULL)
      uiState.menu.close_cb();
  }
}
