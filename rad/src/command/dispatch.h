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

static int8_t target_tool_id;

static void printer_wait_motion(void)
{
  while (plannerMainQueueGetLength()) {
    chThdSleepMilliseconds(50);
  }
}

static void printer_wait_temperature(void)
{
  while (1)
  {
    if (printerIsEstopped())
      return;
    if (curr_command->wait == WAITMODE_All)
    {
      for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++)
      {
        RadTempState s = temperatureGet(machine.extruder.devices[i].temp_id);
        if (!s.target_reached) goto WAIT;
      }
      for (uint8_t i = 0; i < machine.heated_bed.count; i++)
      {
        RadTempState s = temperatureGet(machine.heated_bed.devices[i].temp_id);
        if (!s.target_reached) goto WAIT;
      }
      return;
    } else
    {
      if (target_tool_id >= 0)
      {
        if (curr_command->wait & WAITMODE_CurrentTool)
        {
          RadTempState s = temperatureGet(machine.extruder.devices[target_tool_id].temp_id);
          if (!s.target_reached) goto WAIT;
        } else if (curr_command->wait & WAITMODE_HeatedBed)
        {
          RadTempState s = temperatureGet(machine.heated_bed.devices[target_tool_id].temp_id);
          if (!s.target_reached) goto WAIT;
        }
      }
    }
    return;
    WAIT:
    chThdSleepMilliseconds(250);
  }
}

static void printer_dispatch(void)
{
  if (printerIsEstopped())
    return;

  if (curr_command->t_value >= 0) {
    printer_wait_motion();
    // TODO: Change Tool temp
    // TODO: Motion to offset
    mode.tool = curr_command->t_value;
  }

  if (curr_command->printer.distance)
    mode.distance = curr_command->printer.distance;
  if (curr_command->printer.extruder_distance)
    mode.extruder_distance = curr_command->printer.extruder_distance;
  if (curr_command->printer.feedrate > 0)
    mode.feedrate = curr_command->printer.feedrate;
  if (curr_command->printer.unit)
    mode.unit = curr_command->printer.unit;

  if (!printerIsEstopped())
  {
    switch (curr_command->code)
    {
      case 10104: // Set current tool temp
      case 10109: // Set current tool temp and wait
        if (curr_command->p_value < 0)
        {
          target_tool_id = mode.tool;
        } else if (curr_command->p_value >= 0 || curr_command->p_value < RAD_NUMBER_EXTRUDERS)
        {
          target_tool_id = curr_command->p_value;
        }
        if (target_tool_id >= 0 && target_tool_id < RAD_NUMBER_EXTRUDERS) {
          printer_wait_motion();
          temperatureSet(machine.extruder.devices[target_tool_id].temp_id, curr_command->s_value);
        } else {
          target_tool_id = -1;
        }
        break;
      case 10140: // Set bed temp
      case 10190: // Set bed temp and wait
        if (curr_command->p_value < 0)
        {
          target_tool_id = mode.tool;
        } else if (curr_command->p_value >= 0 || curr_command->p_value < RAD_NUMBER_EXTRUDERS)
        {
          target_tool_id = curr_command->p_value;
        }
        if (target_tool_id >= 0 && target_tool_id < machine.heated_bed.count) {
          printer_wait_motion();
          temperatureSet(machine.heated_bed.devices[target_tool_id].temp_id, curr_command->s_value);
        } else {
          target_tool_id = -1;
        }
        break;
    }
  }

  // Wait Temp
  if (curr_command->wait)
  {
    printer_wait_motion();
    printerSetMessage(L_PRINTER_STATUS_HEATING);
    printer_wait_temperature();
    printerSetMessage(NULL);
  }

  int8_t pwm_id;
  switch (curr_command->code)
  {
    case 4:
      printer_wait_motion();
      chThdSleepMilliseconds(curr_command->p_value);
      break;
    case 28:
      printer_wait_motion();
      commandHoming();
      break;
    case 92:
      commandSetPosition();
      break;
    case 10106: // Fan speed
      pwm_id = -1;
      if (isnan(curr_command->p_value))
      {
        if (mode.tool >= 0 && mode.tool < RAD_NUMBER_EXTRUDERS)
          pwm_id = machine.temperature.devices[machine.extruder.devices[mode.tool].temp_id].cooling_pwm_id;
      } else if (curr_command->p_value >= 0 || curr_command->p_value < machine.fan.count)
      {
        pwm_id = machine.fan.devices[curr_command->p_value].pwm_id;
      }
      if (pwm_id >= 0)
        outputSet(pwm_id, curr_command->s_value);
      break;
  }

  // Motion
  if (!(curr_command->type & COMMANDTYPE_CanHaveAxisWords) ||
      (curr_command->type & COMMANDTYPE_Movement) == COMMANDTYPE_Movement)
    commandMove();

  // TODO: power
  if (curr_command->power)
  {
    printer_wait_motion();
    switch (curr_command->power)
    {
    case POWERMODE_Off:
      break;
    case POWERMODE_Sleep:
      break;
    case POWERMODE_On:
      break;
    case POWERMODE_Idle:
      break;
    }
  }
}
