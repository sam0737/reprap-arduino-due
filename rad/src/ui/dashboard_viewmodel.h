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

static int8_t axis_z_id = -1;

static void ui_dashboard_init(void) {
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    if (machine.kinematics.axes[i].name == AXIS_Z) {
      axis_z_id = i;
      break;
    }
  }
}

static void ui_mainmenu_viewmodel(void);

static void ui_dashboard_viewmodel(void) {
  uiState.renderer = ui_dashboard_renderer;

  const char* message = printerGetMessage();

  if (message == NULL)
  {
    switch (printerGetState())
    {
      case PRINTERSTATE_Standby:
        message = L_UI_STATUS_READY;
        break;
      case PRINTERSTATE_Printing:
      case PRINTERSTATE_Interrupting:
        message = L_UI_STATUS_PRINTING;
        break;
      case PRINTERSTATE_Interrupted:
        message = L_UI_STATUS_INTERRUPTED;
        break;
    }
  }

  bool change_subscreen = FALSE;
  if (uiState.changed_parts & DASHBOARD_Reset)
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

  if (message != uiState.dashboard.status.text)
  {
    uiState.changed_parts |= DASHBOARD_Status;
    uiState.dashboard.status.text = message;
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

  // TODO: layer

  if (axis_z_id >= 0)
  {
    PlannerVirtualPosition pos = plannerGetCurrentPosition();
    if (uiState.dashboard.z_pos != pos.axes[axis_z_id]) {
      uiState.changed_parts |= DASHBOARD_ZPos;
      uiState.dashboard.z_pos = pos.axes[axis_z_id];
    }
  }

  RadStorageHost storage_state = storageGetHostState();
  if (uiState.dashboard.storage_state != storage_state)
  {
    uiState.changed_parts |= DASHBOARD_StorageState;
    uiState.dashboard.storage_state = storage_state;
  }

  for (uint8_t i = 0; i < RAD_NUMBER_TEMPERATURES; i++)
  {
    RadTempState t = temperatureGet(i);
    t.pv += .5;
    t.sv += .5;
    DashboardTempData* data = &uiState.dashboard.temps[i];
    if (data->pv != t.pv) {
      uiState.changed_parts |= DASHBOARD_Temperatures;
      data->pv = t.pv;
    }
    if (data->sv != t.sv) {
      uiState.changed_parts |= DASHBOARD_Temperatures;
      data->sv = t.sv;
    }

    DashboardTempState state =
      outputGet(machine.temperature.devices[i].heating_pwm_id) > 0 ? DASHBOARD_Temp_Heating :
      outputGet(machine.temperature.devices[i].cooling_pwm_id) > 0 ? DASHBOARD_Temp_Cooling :
        DASHBOARD_Temp_Idle;
    if (data->state != state)
    {
      uiState.changed_parts |= DASHBOARD_Temperatures;
      data->state = state;
    }
  }

  if (change_subscreen &&
      RAD_NUMBER_EXTRUDERS + machine.heated_bed.count + machine.temp_monitor.count > DISPLAY_DASHBOARD_TEMPS) {
    uiState.changed_parts |= DASHBOARD_Temperatures;
    uiState.dashboard.temps_screen += DISPLAY_DASHBOARD_TEMPS;
    if (uiState.dashboard.temps_screen >=
        RAD_NUMBER_EXTRUDERS + machine.heated_bed.count + machine.temp_monitor.count)
    {
      uiState.dashboard.temps_screen = 0;
    }
  }

  // TODO: time_spent
  uint8_t active_extruder = printerGetActiveExtruder();
  if (uiState.dashboard.active_extruder != active_extruder)
  {
    uiState.changed_parts |= DASHBOARD_ActiveExtruder;
    uiState.dashboard.active_extruder = active_extruder;
  }

  // TODO: 2 Line Change
}
