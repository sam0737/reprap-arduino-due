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

static int8_t axis_ids[];
static int8_t joint_ids[];

static void _draw_clock(coord_t x, coord_t y)
{
  gdispFillCircle(x, y, 11, Fg);
  gdispFillCircle(x, y, 9, Bg);
  gdispFillArea(x - 1, y - 8, 2, 9, Fg);
  gdispFillArea(x + 1, y - 1, 6, 2, Fg);
}

static void _draw_card(coord_t x, coord_t y, RadStorageHost state)
{
  if (state == STORAGE_None)
  {
    gdispFillArea(x, y, 23, 28, Bg);
  } else
  {
    gdispFillConvexPoly(x, y,
        (point[]) {
          { .x = 6, .y = 0 },
          { .x = 23, .y = 0 },
          { .x = 23, .y = 28 },
          { .x = 0, .y = 28 },
          { .x = 0, .y = 6 },
        },
        5,
        state == STORAGE_Usb ? HTML2COLOR(0x2b4cc2) : Fg
    );
    gdispFillArea(x + 2, y + 5, 3, 6, Bg);
    gdispFillArea(x + 6, y + 1, 3, 6, Bg);
    gdispFillArea(x + 10, y + 1, 3, 6, Bg);
    gdispFillArea(x + 14, y + 1, 3, 6, Bg);
    gdispFillArea(x + 18, y + 1, 3, 6, Bg);

    if (state == STORAGE_Usb) {
      gdispDrawStringBox(x, y + 7, 23, 22, "U", fontSmall, Bg, justifyCenter);
    }
  }
}

static void _draw_temp(coord_t x, coord_t y, DashboardTempData *data)
{
  gdispFillStringBox(
      x + 35, y + 4,
      50, 28,
      itostr3left(data->pv), fontText, HighlightFg, AuxBg, justifyLeft);
  gdispFillStringBox(
      x + 48, y + 27,
      37, 18,
      itostr3left(data->sv), fontSmall, HighlightFg, AuxBg, justifyLeft);

  gdispFillConvexPoly(x, y,
        (point[]) {
          { .x = 39, .y = 29 },
          { .x = 47, .y = 34 },
          { .x = 39, .y = 39 }
        },
        3,
        HighlightFg
    );
}

static void _draw_extruder(char* text, coord_t x, coord_t y, bool_t active, DashboardTempData *data)
{
  gdispFillArea(x + 3, y + 28, 32, 18, data->state == DASHBOARD_Temp_Heating ? TempHeating : TempIdle);
  gdispFillArea(x + 11, y, 16, 30, TempBorder);
  gdispDrawStringBox(x + 3, y + 5, 32, 22, text, fontSmall, active ? TempHigh : TempLow, justifyCenter);

  float value =
      (fmin(fmax(data->pv, TempLowPoint), TempHighPoint) - TempLowPoint) /
      (TempHighPoint - TempLowPoint);
  gdispFillConvexPoly(x + 3, y,
      (point[]) {
        { .x = 2, .y = 30 },
        { .x = 30, .y = 30 },
        { .x = 30, .y = 44 },
        { .x = 24, .y = 44 },
        { .x = 18, .y = 55 },
        { .x = 14, .y = 55 },
        { .x = 8, .y = 44 },
        { .x = 2, .y = 44 },
      },
      8,
      RGB2COLOR(
          (uint8_t) ((int)(RED_OF(TempHigh) - RED_OF(TempLow)) * value + RED_OF(TempLow)),
          (uint8_t) ((int)(GREEN_OF(TempHigh) - GREEN_OF(TempLow)) * value + GREEN_OF(TempLow)),
          (uint8_t) ((int)(BLUE_OF(TempHigh) - BLUE_OF(TempLow)) * value + BLUE_OF(TempLow))
      )
  );
  _draw_temp(x, y, data);
}

static void _draw_heated_bed(coord_t x, coord_t y, DashboardTempData *data)
{
  gdispFillArea(x + 3, y + 37, 32, 7, data->state == DASHBOARD_Temp_Heating ? TempHeating : TempIdle);

  float value =
      (fmin(fmax(data->pv, TempLowPoint), TempHighPoint) - TempLowPoint) /
      (TempHighPoint - TempLowPoint);

  gdispFillConvexPoly(x + 3, y,
      (point[]) {
        { .x = 0, .y = 37 },
        { .x = 6, .y = 30 },
        { .x = 33-6, .y = 30 },
        { .x = 33, .y = 37 }
      },
      4,
      RGB2COLOR(
          (uint8_t) ((int)(RED_OF(TempHigh) - RED_OF(TempLow)) * value + RED_OF(TempLow)),
          (uint8_t) ((int)(GREEN_OF(TempHigh) - GREEN_OF(TempLow)) * value + GREEN_OF(TempLow)),
          (uint8_t) ((int)(BLUE_OF(TempHigh) - BLUE_OF(TempLow)) * value + BLUE_OF(TempLow))
      )
  );
  _draw_temp(x, y, data);
}

static void _draw_generic_temp(coord_t x, coord_t y, DashboardTempData *data)
{
  gdispFillArea(x + 12, y + 3, 12, 26, TempBorder);
  gdispFillCircle(x + 18, y + 3 + 25 + 8, 9, TempBorder);

  float value =
      (fmin(fmax(data->pv, TempLowPoint), TempHighPoint) - TempLowPoint) /
      (TempHighPoint - TempLowPoint);

  color_t c = RGB2COLOR(
      (uint8_t) ((int)(RED_OF(TempHigh) - RED_OF(TempLow)) * value + RED_OF(TempLow)),
      (uint8_t) ((int)(GREEN_OF(TempHigh) - GREEN_OF(TempLow)) * value + GREEN_OF(TempLow)),
      (uint8_t) ((int)(BLUE_OF(TempHigh) - BLUE_OF(TempLow)) * value + BLUE_OF(TempLow))
  );

  gdispFillArea(x + 12 + 3, y + 6, 6, 26, c);
  gdispFillCircle(x + 18, y + 3 + 25 + 8, 6, c);

  _draw_temp(x, y, data);
}

static void _draw_axis(char* text, coord_t sx, coord_t sy, coord_t dx, coord_t dy,
    DashboardAxis *axis, float axis_min, float axis_max)
{
  float value;
  if (axis->pos < axis_min) {
    value = 0;
  } else if (axis->pos > axis_max) {
    value = 1;
  } else {
    value = (axis->pos - axis_min) / (axis_max - axis_min);
  }

  sx = (dx - sx) * value + sx;
  sy = (dy - sy) * value + sy;
  gdispFillCircle(sx, sy, 9,
      axis->limit_state != LIMIT_Normal ? LimitBg :
      axis->homed ? HomedBg :
      Bg);
  gdispDrawStringBox(sx - 10, sy - 8, 20, 18, text, fontSmall, Fg, justifyCenter);
}

static void _dashboard_render_temp(uint8_t index)
{
  coord_t pos = index;
  index += uiState.dashboard.temps_screen;
  if (index < RAD_NUMBER_EXTRUDERS)
  {
    char text[2] = "0";
    text[0] = index + '1';
    _draw_extruder(text, pos * 80, 180,
        index == uiState.dashboard.active_extruder, &uiState.dashboard.temps[machine.extruder.devices[index].temp_id]);
  }

  index -= RAD_NUMBER_EXTRUDERS;
  if (index < machine.heated_bed.count)
    _draw_heated_bed(pos * 80, 180, &uiState.dashboard.temps[machine.heated_bed.devices[index].temp_id]);

  index -= machine.heated_bed.count;
  if (index < machine.temp_monitor.count)
    _draw_generic_temp(pos * 80, 180, &uiState.dashboard.temps[machine.temp_monitor.devices[index].temp_id]);
}

static void ui_dashboard_renderer(void) {
  if (uiState.changed_parts & DASHBOARD_Reset)
  {
    gdispClear(Bg);
    gdispFillArea(0, DISPLAY_HEIGHT - 75, DISPLAY_WIDTH, 5, AuxBorder);
    _draw_clock(20, 141);
  }

  if ((uiState.changed_parts & DASHBOARD_Status) &&
      uiState.dashboard.status.text != 0)
  {
    if (uiState.dashboard.status.marquee.length == 0) {
      uiState.dashboard.status.marquee.length = gdispGetStringWidth(uiState.dashboard.status.text, fontTitle);
      uiState.dashboard.status.marquee.length -= (DISPLAY_WIDTH - 10) - 1;
    }
    gdispFillArea(0, 0, 5, 50, uiState.dashboard.status.estopped ? HighlightBg2 : HighlightBg);
    gdispFillArea(DISPLAY_WIDTH - 5, 0, 5, 50, uiState.dashboard.status.estopped ? HighlightBg2 : HighlightBg);
    gdispFillStringBoxWithOffset(
        5, 0,
        DISPLAY_WIDTH - 10, 50,
        (
            uiState.dashboard.status.marquee.offset <= 0 ? 0 :
            uiState.dashboard.status.marquee.offset > uiState.dashboard.status.marquee.length ? -uiState.dashboard.status.marquee.length :
            -uiState.dashboard.status.marquee.offset
        ),
        uiState.dashboard.status.text, fontTitle, HighlightFg,
        uiState.dashboard.status.estopped ? HighlightBg2 : HighlightBg);
  }

  if ((uiState.changed_parts & DASHBOARD_Source))
  {
    if (uiState.dashboard.source == PRINTINGSOURCE_Storage)
    {
      gdispFillStringBox(
          10, 63,
          160, 28,
          uiState.filename, fontText, Fg, Bg, justifyLeft);
    } else
    {
      gdispFillArea(10, 63, 160, 60, Bg);
    }
  }
  if (uiState.dashboard.source == PRINTINGSOURCE_Storage)
  {
    if ((uiState.changed_parts & DASHBOARD_Progress))
    {
      gdispDrawLine(10, 96, 170, 96, Fg);
      gdispDrawLine(10, 116, 170, 116, Fg);
      gdispDrawLine(10, 96, 10, 116, Fg);
      gdispDrawLine(170, 96, 170, 116, Fg);

      coord_t w = (157 * uiState.dashboard.progress.processed / uiState.dashboard.progress.total) + 1;
      gdispFillArea(10 + 1, 96 + 1, w, 19, HighlightBg);
    }
  }

  if ((uiState.changed_parts & DASHBOARD_TimeSpent))
  {
    char text[10] = "----";
    if (uiState.dashboard.time_spent >= 0)
    {
      snprintf(text, 10, "%dh %dm", uiState.dashboard.time_spent / 60, uiState.dashboard.time_spent % 60);
    }
    gdispFillStringBox(
        40, 126,
        100, 30,
        text, fontSmall, Fg, Bg, justifyLeft);
  }

  if ((uiState.changed_parts & DASHBOARD_StorageState))
  {
    _draw_card(146, 127, uiState.dashboard.storage_state);
  }

  if ((uiState.changed_parts & DASHBOARD_Pos))
  {
    gdispFillArea(200 - 10, 68, 20, 64, Bg);
    gdispFillArea(218 - 10, 55, 40, 72, Bg);
    gdispFillArea(211, 137 - 10, 104, 20, Bg);
    if (axis_ids[2] > 0)
    {
      gdispDrawLine(200, 130, 200, 70, AuxBorder);
      _draw_axis("Z", 200, 122, 200, 78,
          &uiState.dashboard.axes[2],
          machine.kinematics.axes[axis_ids[2]].min_limit, machine.kinematics.axes[axis_ids[2]].max_limit);
      gdispFillStringBox(
          165, 50,
          70, 18,
          ftostr42best(uiState.dashboard.axes[2].pos), fontSmall, Fg, Bg, justifyCenter);
    }
    if (axis_ids[1] > 0)
    {
      gdispDrawLine(218, 126, 248, 56, AuxBorder);
      _draw_axis("Y", 222, 119, 244, 65,
          &uiState.dashboard.axes[1],
          machine.kinematics.axes[axis_ids[1]].min_limit, machine.kinematics.axes[axis_ids[1]].max_limit);
      gdispFillStringBox(
          253, 80,
          70, 18,
          ftostr42best(uiState.dashboard.axes[1].pos), fontSmall, Fg, Bg, justifyLeft);
    }
    if (axis_ids[2] > 0)
    {
      gdispDrawLine(213, 137, 313, 137, AuxBorder);
      _draw_axis("X", 221, 137, 305, 137,
          &uiState.dashboard.axes[0],
          machine.kinematics.axes[axis_ids[0]].min_limit, machine.kinematics.axes[axis_ids[0]].max_limit);
      gdispFillStringBox(
          225, 146,
          70, 18,
          ftostr42best(uiState.dashboard.axes[0].pos), fontSmall, Fg, Bg, justifyCenter);
    }
  }

  if ((uiState.changed_parts & DASHBOARD_TemperaturesChangeScreen))
  {
    gdispFillArea(0, DISPLAY_HEIGHT - 70, DISPLAY_WIDTH, 70, AuxBg);
  }

  if ((uiState.changed_parts & (DASHBOARD_Temperatures | DASHBOARD_ActiveExtruder)))
  {
    for (uint8_t i = 0; i < DISPLAY_DASHBOARD_TEMPS; i++)
    {
      _dashboard_render_temp(i);
    }
  }
}

/** @} */
