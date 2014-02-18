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

static void ui_dashboard_render_temp(uint8_t temp_index_id);

static void ui_dashboard_renderer(void) {
  if (uiState.changed_parts & DASHBOARD_Reset)
  {
    tdispClear();
  }

  // Status Line
  if ((uiState.changed_parts & DASHBOARD_Status) &&
      uiState.dashboard.status.text != 0)
  {
    if (uiState.dashboard.status.marquee.length == 0) {
      uiState.dashboard.status.marquee.length = strlen(uiState.dashboard.status.text) - TDISP_COLUMNS;
    }

    tdispSetCursor(0, 0);
    uint8_t offset =
        uiState.dashboard.status.marquee.offset <= 0 ? 0 :
        uiState.dashboard.status.marquee.offset > uiState.dashboard.status.marquee.length ? uiState.dashboard.status.marquee.length :
        uiState.dashboard.status.marquee.offset;
    for (uint8_t i = 0; i < TDISP_COLUMNS; i++)
    {
      int16_t pos = i + offset;
      tdispDrawChar(
          pos < uiState.dashboard.status.marquee.length + TDISP_COLUMNS ?
          uiState.dashboard.status.text[pos] :
          ' ');
    }
  }

  // Sub Status
  if (TDISP_ROWS >= 4)
  {
    if (uiState.changed_parts & DASHBOARD_TimeSpent)
    {
      tdispSetCursor(0, 2);
      tdispDrawChar('T');
      tdispDrawString(itostr2(uiState.dashboard.time_spent / 60));
      tdispDrawChar(':');
      tdispDrawString(itostr2(uiState.dashboard.time_spent % 60));
    }

    if (uiState.changed_parts & DASHBOARD_ZPos)
    {
      tdispSetCursor(7, 2);
      tdispDrawChar('Z');
      tdispDrawString(ftostr42(uiState.dashboard.z_pos));
    }

    if (uiState.changed_parts & DASHBOARD_StorageState)
    {
      tdispSetCursor(TDISP_COLUMNS - 1, 2);
      tdispDrawChar(
          uiState.dashboard.storage_state == STORAGE_Usb ? 'U' :
          uiState.dashboard.storage_state == STORAGE_Local ? 'L' :
              ' ');
    }
  }

  // Temperatures
  if (uiState.changed_parts & (DASHBOARD_Temperatures | DASHBOARD_ActiveExtruder))
  {
    for (uint8_t i = 0; i < DISPLAY_DASHBOARD_TEMPS; i++)
    {
      tdispSetCursor(i * (TDISP_COLUMNS >= 20 ? 10 : 8), TDISP_ROWS == 2 ? 1 : 3);
      ui_dashboard_render_temp(i + uiState.dashboard.temps_screen);
    }
  }
}

static void ui_dashboard_render_temp_core(char prefix, DashboardTempData* data)
{
  tdispDrawChar(prefix);
  tdispDrawString(itostr3(data->pv));
  tdispDrawChar('>');
  tdispDrawString(itostr3left(data->sv));
  if (TDISP_COLUMNS >= 20) {
    tdispDrawString("C");
  }
  tdispDrawString(itospace(3, data->sv));
}

static void ui_dashboard_render_temp(uint8_t temp_index_id)
{
  if (temp_index_id < RAD_NUMBER_EXTRUDERS) {
    ui_dashboard_render_temp_core('E', &uiState.dashboard.temps[machine.extruder.devices[temp_index_id].temp_id]);
    return;
  }
  temp_index_id -= RAD_NUMBER_EXTRUDERS;
  if (temp_index_id < machine.heated_bed.count) {
    ui_dashboard_render_temp_core('H', &uiState.dashboard.temps[machine.heated_bed.devices[temp_index_id].temp_id]);
    return;
  }
  temp_index_id -= machine.heated_bed.count;
  if (temp_index_id < machine.temp_monitor.count) {
    ui_dashboard_render_temp_core('M', &uiState.dashboard.temps[machine.temp_monitor.devices[temp_index_id].temp_id]);
    return;
  }
}
