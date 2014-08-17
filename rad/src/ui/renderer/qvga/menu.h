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
    gdispClear(White);
  }

  if (uiState.changed_parts & MENU_Changed)
  {
    for (uint8_t i = 0; i <= DISPLAY_MENU_LINE_HEIGHT; i++)
    {
      const UiMenuItem* item =
          i > 0 ? uiState.menu.get_next_cb() :
          uiMenuGetItem(i);
      char suffix[2] = " ";
      gdispFillStringBox(
          5, DISPLAY_MENU_LINE_HEIGHT * i,
          GDISP_SCREEN_WIDTH - 10, DISPLAY_MENU_LINE_HEIGHT,
          item != NULL ? item->name : "", fontText,
          i == uiState.menu.pos ? White : Black,
          i == uiState.menu.pos ? Black : White,
          justifyLeft);
      if (item)
        suffix[0] = item->suffix;
      gdispDrawStringBox(
          5, DISPLAY_MENU_LINE_HEIGHT * i,
          GDISP_SCREEN_WIDTH - 10, DISPLAY_MENU_LINE_HEIGHT,
          suffix, fontText,
          i == uiState.menu.pos ? White : Black,
          justifyRight);
    }
    if (uiState.menu.close_cb != NULL)
      uiState.menu.close_cb();
  }
}
