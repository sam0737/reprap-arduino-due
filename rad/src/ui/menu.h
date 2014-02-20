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

const UiMenuItem* uiStandardMenuGet(int16_t index)
{
  if (index < 0 || index >= ((UiStandardMenu*)uiState.menu.state)->count)
    return NULL;
  return &((UiStandardMenu*)uiState.menu.state)->menus[index];
}

int16_t uiStandardMenuCount()
{
  return ((UiStandardMenu*)uiState.menu.state)->count;
}

const UiMenuItem* uiMenuGetItem(int8_t offset)
{
  const UiMenuItem* ret;

  int16_t count = uiState.menu.count_cb();

  if (uiState.menu.current >= count)
  {
    // Out of range: overflow
    uiState.menu.current = count - 1;
    if (uiState.menu.pos >= count)
      uiState.menu.pos = count - 1;
  }
  if (uiState.menu.current < 0) {
    // Out of range: underflow
    uiState.menu.current = 0;
  }

  offset -= uiState.menu.pos;

  if (uiState.menu.get_next_cb)
    return uiState.menu.get_cb(uiState.menu.current + offset);

  int16_t index = uiState.menu.current;

  // If the current highlighted item is invisible, move to next
  while (1)
  {
    ret = uiState.menu.get_cb(index);
    if (ret == NULL) break;
    if (!ret->visible_cb || ret->visible_cb())
      goto OK;
    index += uiState.menu.offset > 0 ? 1 : -1;
  }

  index = uiState.menu.current;
  while (1)
  {
    ret = uiState.menu.get_cb(index);
    if (ret == NULL) return NULL;
    if (!ret->visible_cb || ret->visible_cb())
      goto OK;
    index -= uiState.menu.offset > 0 ? 1 : -1;
  }

  OK:
  uiState.menu.current = index;

  // If the current highlighted item is invisible, move to next
  while (offset < 0)
  {
    if (index < 1) return NULL;
    ret = uiState.menu.get_cb(index - 1);
    if (ret == NULL) return NULL;
    if (!ret->visible_cb || ret->visible_cb()) {
      offset++;
    }
    index--;
  }
  while (offset > 0)
  {
    ret = uiState.menu.get_cb(index + 1);
    if (ret == NULL) return NULL;
    if (!ret->visible_cb || ret->visible_cb()) {
      offset--;
    }
    index++;
  }
  return uiState.menu.get_cb(index);
}

uint8_t ui_menu_shows_back_item(void)
{
  return !machine.ui.back_button.enabled;
}

static void ui_dashboard_viewmodel(void);
static void ui_menu_goto_dashboard(void *state)
{
  uiChangePage(ui_dashboard_viewmodel);
}

static void ui_mainmenu_viewmodel(void);
static void ui_menu_goto_mainmenu(void *state)
{
  uiChangePage(ui_mainmenu_viewmodel);
}

void ui_menu_goto_page(void* state)
{
  uiChangePage((display_viewmodel_t)state);
}

