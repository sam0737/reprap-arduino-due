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

static int8_t axis_ids[3] = {-1, -1, -1};
static int8_t joint_ids[3] = {-1, -1, -1};

static void ui_mainmenu_viewmodel(void);

static void ui_dashboard_viewmodel_core(void)
{
  uiState.renderer = ui_dashboard_renderer;

  bool reset = uiState.changed_parts & DASHBOARD_Reset;
  bool change_subscreen = FALSE;
  if (reset)
  {
    FLUSH_INPUT(enter_button);
    uiState.dashboard.subscreen_time = chTimeNow() / MS2ST(1200);
  } else {
    if (GET_INPUT(enter_button).button.times) {
      uiChangePage(ui_mainmenu_viewmodel);
      return;
    }
    systime_t subscreen_time = chTimeNow() / MS2ST(1200);
    if (uiState.dashboard.subscreen_time != subscreen_time)
    {
      uiState.dashboard.subscreen_time = subscreen_time;
      change_subscreen = TRUE;
    }
  }

  {
    uint8_t version;
    version = printerGetMessage(uiState.dashboard.status.version,
        uiState.dashboard.status.text, sizeof(uiState.dashboard.status.text));

    if (reset || version != uiState.dashboard.status.version)
    {
      uiState.changed_parts |= DASHBOARD_Status;
      uiState.dashboard.status.version = version;
      uiState.dashboard.status.estopped = printerIsEstopped();
      uiState.dashboard.status.marquee.time = chTimeNow();
      uiState.dashboard.status.marquee.length = 0;
      uiState.dashboard.status.marquee.offset = 0;
    }
    else if (uiState.dashboard.status.marquee.length > 0)
    {
      int16_t new_offset =
        (chTimeNow() - uiState.dashboard.status.marquee.time) / DISPLAY_MARQUEE_DELAY %
            (uiState.dashboard.status.marquee.length + (MS2ST(3000) / DISPLAY_MARQUEE_DELAY))
            - (MS2ST(1500) / DISPLAY_MARQUEE_DELAY);

      if (new_offset < uiState.dashboard.status.marquee.offset ||
          (new_offset >= 0 && uiState.dashboard.status.marquee.offset <= uiState.dashboard.status.marquee.length))
      {
        uiState.changed_parts |= DASHBOARD_Status;
        uiState.dashboard.status.marquee.offset = new_offset;
      }
    }
  }

  {
    PrintingSource source = printerGetMainSource();
    if (reset || uiState.dashboard.source != source)
    {
      uiState.changed_parts |= DASHBOARD_Source | DASHBOARD_Progress;
      uiState.dashboard.source = source;
      if (source == PRINTINGSOURCE_Storage)
        uiState.dashboard.progress.total = dataStorageGetCurrentFileSize();
    }
    if (source == PRINTINGSOURCE_Storage)
    {
      uint32_t processed = dataStorageGetCurrentProcessed();
      if (uiState.dashboard.progress.processed != processed) {
        uiState.changed_parts |= DASHBOARD_Progress;
        uiState.dashboard.progress.processed = processed;
      }
    }
  }

  {
    int32_t time_spent = printerTimeSpent();
    if (time_spent < 0)
    {
      if (uiState.dashboard.time_spent >= 0)
      {
        uiState.changed_parts |= DASHBOARD_TimeSpent;
        uiState.dashboard.time_spent = -1;
      }
    } else
    {
      time_spent /= 60;
      if (uiState.dashboard.time_spent != time_spent)
      {
        uiState.changed_parts |= DASHBOARD_TimeSpent;
        uiState.dashboard.time_spent = time_spent;
      }
    }
  }

  {
    PlannerVirtualPosition pos = stepperGetCurrentPosition();
    RadJointsState joints_state = stepperGetJointsState();
    for (uint8_t i = 0; i < 3; i++)
    {
      if (axis_ids[i] >= 0)
      {
        if (uiState.dashboard.axes[i].pos != pos.axes[axis_ids[i]]) {
          uiState.changed_parts |= DASHBOARD_Pos;
          uiState.dashboard.axes[i].pos = pos.axes[axis_ids[i]];
        }
      }
      if (joint_ids[i] >= 0)
      {
        if (uiState.dashboard.axes[i].homed != joints_state.joints[joint_ids[i]].homed) {
          uiState.changed_parts |= DASHBOARD_Pos;
          uiState.dashboard.axes[i].homed = joints_state.joints[joint_ids[i]].homed;
        }
        if (uiState.dashboard.axes[i].limit_state != joints_state.joints[joint_ids[i]].limit_state) {
          uiState.changed_parts |= DASHBOARD_Pos;
          uiState.dashboard.axes[i].limit_state = joints_state.joints[joint_ids[i]].limit_state;
        }
      }
    }
  }

  {
    RadStorageHost storage_state = storageGetHostState();
    if (uiState.dashboard.storage_state != storage_state)
    {
      uiState.changed_parts |= DASHBOARD_StorageState;
      uiState.dashboard.storage_state = storage_state;
    }
  }

  for (uint8_t i = 0; i < RAD_NUMBER_TEMPERATURES; i++)
  {
    RadTempState t = temperatureGet(i);
    t.pv += .5;
    t.sv += .5;
    DashboardTempData* data = &uiState.dashboard.temps[i];
    data->changed = FALSE;
    if (data->pv != (int16_t) t.pv) {
      data->changed = TRUE;
      uiState.changed_parts |= DASHBOARD_Temperatures;
      data->pv = t.pv;
    }
    if (data->sv != (int16_t) t.sv) {
      data->changed = TRUE;
      uiState.changed_parts |= DASHBOARD_Temperatures;
      data->sv = t.sv;
    }

    DashboardTempState state =
      outputGet(machine.temperature.devices[i].heating_pwm_id) > 0 ? DASHBOARD_Temp_Heating :
      outputGet(machine.temperature.devices[i].cooling_pwm_id) > 0 ? DASHBOARD_Temp_Cooling :
        DASHBOARD_Temp_Idle;
    if (data->state != state)
    {
      data->changed = TRUE;
      uiState.changed_parts |= DASHBOARD_Temperatures;
      data->state = state;
    }
  }

  if (change_subscreen &&
      RAD_NUMBER_EXTRUDERS + machine.heated_bed.count + machine.temp_monitor.count > DISPLAY_DASHBOARD_TEMPS)
  {
    uiState.changed_parts |= DASHBOARD_Temperatures | DASHBOARD_TemperaturesChangeScreen;
    uiState.dashboard.temps_screen += DISPLAY_DASHBOARD_TEMPS;
    if (uiState.dashboard.temps_screen >=
        RAD_NUMBER_EXTRUDERS + machine.heated_bed.count + machine.temp_monitor.count)
    {
      uiState.dashboard.temps_screen = 0;
    }
  }

  {
    uint8_t active_extruder = printerGetActiveExtruder();
    if (uiState.dashboard.active_extruder != active_extruder)
    {
      uiState.changed_parts |= DASHBOARD_ActiveExtruder;
      uiState.dashboard.active_extruder = active_extruder;
    }
  }

  // TODO: 2 Line Change
}

static void ui_dashboard_viewmodel(void) {
  uiState.viewmodel = ui_dashboard_viewmodel_core;
  uiState.dashboard.status.version = 0;
}
