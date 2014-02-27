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
  if (index < 0 || index >= uiState.menu.standard.menu->count)
    return NULL;
  for (int8_t i = 0; i < uiState.menu.standard.menu->count; i++)
  {
    const UiMenuItem* item = &uiState.menu.standard.menu->menus[i];
    if (!item->visible_cb || item->visible_cb())
    {
      if (index == 0) {
        uiState.menu.standard.last_index = i;
        return item;
      }
      index--;
    }
  }
  return NULL;
}

const UiMenuItem* uiStandardMenuGetNext(void)
{
  for (int8_t i = uiState.menu.standard.last_index + 1; i < uiState.menu.standard.menu->count; i++)
  {
    const UiMenuItem* item = &uiState.menu.standard.menu->menus[i];
    if (!item->visible_cb || item->visible_cb())
    {
      uiState.menu.standard.last_index = i;
      return item;
    }
  }
  return NULL;
}

int16_t uiStandardMenuCount()
{
  int8_t count = 0;
  for (int8_t i = uiState.menu.standard.menu->count - 1; i >= 0; i--)
  {
    const UiMenuItem* item = &uiState.menu.standard.menu->menus[i];
    if (!item->visible_cb || item->visible_cb())
      count++;
  }
  return count;
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
  if (uiState.menu.current < 0)
  {
    // Out of range: underflow
    uiState.menu.current = 0;
  }

  return uiState.menu.get_cb(uiState.menu.current + offset - uiState.menu.pos);
}

uint8_t ui_menu_shows_back_item(void)
{
  return !machine.ui.back_button.enabled;
}

static void ui_dashboard_viewmodel(void);
static void ui_menu_goto_dashboard(void)
{
  uiChangePage(ui_dashboard_viewmodel);
}

static void ui_mainmenu_viewmodel(void);
static void ui_menu_goto_mainmenu(void)
{
  uiChangePage(ui_mainmenu_viewmodel);
}

void ui_menu_goto_page(void* state)
{
  uiChangePage((display_viewmodel_t)state);
}

