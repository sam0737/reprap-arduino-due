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

static void ui_dashboard_renderer(void) {
  if (uiState.changed_parts & DASHBOARD_Reset)
  {
    gdispClear(White);
  }
  if ((uiState.changed_parts & DASHBOARD_Status) &&
      uiState.dashboard.status.text != 0)
  {
    if (uiState.dashboard.status.marquee.length == 0) {
      uiState.dashboard.status.marquee.length = gdispGetStringWidth(uiState.dashboard.status.text, fontText);
      uiState.dashboard.status.marquee.length -= (GDISP_SCREEN_WIDTH - 10) - 1;
    }
    gdispFillStringBoxWithOffset(
        5, 0,
        GDISP_SCREEN_WIDTH - 10, GDISP_SCREEN_HEIGHT / 8,
        (
            uiState.dashboard.status.marquee.offset <= 0 ? 0 :
            uiState.dashboard.status.marquee.offset > uiState.dashboard.status.marquee.length ? -uiState.dashboard.status.marquee.length :
            -uiState.dashboard.status.marquee.offset
        ),
        uiState.dashboard.status.text, fontText, Black, White);
  }
}

/** @} */
