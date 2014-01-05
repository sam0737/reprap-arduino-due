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

static PlannerVirtualPosition commanded;
static uint8_t active_extruder;
static float feedrate;
static float feedrate_multiplier = 1;

static bool_t is_axis_relative;
static bool_t is_extruder_relative;

static void commandMove(void)
{
  bool_t axis_involved = FALSE;
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    if (code_seen(machine.kinematics.axes[i].name)) {
      axis_involved = TRUE;
      if (is_axis_relative)
        commanded.axes[i] += code_value();
      else
        commanded.axes[i] = code_value();
    }
  }
  if (code_seen('E'))
  {
    if (is_extruder_relative)
      commanded.extruders[active_extruder] += code_value();
    else
      commanded.extruders[active_extruder] = code_value();
  }
  if(code_seen('F')) {
    float next_feedrate = code_value();
    if (next_feedrate > 0.0)
      feedrate = next_feedrate;
  }
  plannerAddAxisPoint(
      &commanded,
      axis_involved ? feedrate * feedrate_multiplier : feedrate);
}

static void commandSetPosition(void)
{
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    if (code_seen(machine.kinematics.axes[i].name)) {
        commanded.axes[i] = code_value();
    }
  }
  if (code_seen('E'))
  {
    commanded.extruders[active_extruder] = code_value();
  }
  plannerSetPosition(&commanded);
}

static void printerSyncCommanded(void)
{
  commanded = plannerSyncCurrentPosition();
}
