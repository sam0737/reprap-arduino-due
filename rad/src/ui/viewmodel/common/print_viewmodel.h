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

static void ui_print_viewmodel_core(void);

uint8_t dir_depth = 0;

void ui_print_up(void)
{
  if (dir_depth == 0)
  {
    uiChangePage(ui_mainmenu_viewmodel);
    return;
  }

  dir_depth--;
  storageChangeDir("..");
  uiChangePage(ui_print_viewmodel_core);
}

void ui_print_up_action(void* state)
{
  (void)state;
  ui_print_up();
}

void ui_print_change_directory(void* filename)
{
  dir_depth++;
  storageChangeDir((char*)filename);
  uiChangePage(ui_print_viewmodel_core);
}

void ui_print_file(void* filename)
{
  dataStorageSelect((char*)filename);
  dataStoragePrintStart();
  uiChangePage(ui_dashboard_viewmodel);
}

static const UiMenuItem* ui_print_file_to_menu(RadFileInfo *file)
{
  uiState.menu.print.item = (UiMenuItem)
    {
      .name = file->filename,
      .action_cb = file->type == FILETYPE_Directory ?
          ui_print_change_directory :
          ui_print_file,
      .suffix = file->type == FILETYPE_Directory ? '>' : 0,
      .state = file->filename
    };
  return &uiState.menu.print.item;
}

int16_t ui_print_count(void)
{
  int16_t i = 0;
  RadFileInfo file;

  storageOpenDir();
  for (;;)
  {
    if (!storageFetchFileInfo(&file))
      break;

    if (!(strcmp(file.filename, ".") == 0 ||
        strcmp(file.filename, "..") == 0))
      i++;
  }

  storageCloseDir();
  return (i < 1 ? 1 : i) +
      (ui_menu_shows_back_item() || dir_depth != 0);
}

const UiMenuItem* ui_print_get(int16_t index)
{
  if (ui_menu_shows_back_item() || dir_depth != 0)
  {
    if (index == 0)
    {
      uiState.menu.print.item = (UiMenuItem)
        {
            .name = dir_depth == 0 ? L_UI_BACK : L_UI_PRINT_UP,
            .suffix = '^',
            .action_cb = ui_print_up_action
        };
      return &uiState.menu.print.item;
    }
    index--;
  }

  uiState.menu.print.directory_opened = -1;
  storageOpenDir();

  RadFileInfo file;
  for (int16_t i = 0; i <= index; i++)
  {
    do {
      if (!storageFetchFileInfo(&file)) {
        if (i == 0 && index == 0) {
          uiState.menu.print.item = (UiMenuItem)
            {
                .name = L_UI_PRINT_NO_FILES
            };
          return &uiState.menu.print.item;
        }
        return NULL;
      }
    } while (
        strcmp(file.filename, ".") == 0 ||
        strcmp(file.filename, "..") == 0);
  }

  return ui_print_file_to_menu(&file);
}

static const UiMenuItem* ui_print_get_next(void)
{
  if (uiState.menu.print.directory_opened >= 0)
  {
    uiState.menu.print.directory_opened++;
    return ui_print_get(uiState.menu.current - uiState.menu.pos + uiState.menu.print.directory_opened);
  }

  RadFileInfo file;
  do {
    if (!storageFetchFileInfo(&file))
      return NULL;
  } while (
      strcmp(file.filename, ".") == 0 ||
      strcmp(file.filename, "..") == 0);

  return ui_print_file_to_menu(&file);
}

static void ui_print_close(void)
{
  if (uiState.menu.print.directory_opened >= 0)
    return;
  storageCloseDir();
  uiState.menu.print.directory_opened = 0;
}

static void ui_print_viewmodel_core(void) {
  uiState.viewmodel = ui_menu_viewmodel;
  uiState.menu.get_cb = ui_print_get;
  uiState.menu.count_cb = ui_print_count;
  uiState.menu.get_next_cb = ui_print_get_next;
  uiState.menu.close_cb = ui_print_close;
  uiState.menu.back_cb = dir_depth == 0 ?
      ui_menu_goto_mainmenu :
      ui_print_up;
}

static void ui_print_viewmodel(void) {
  ui_print_viewmodel_core();
}
