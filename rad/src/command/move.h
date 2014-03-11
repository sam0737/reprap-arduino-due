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
static float feedrate_multiplier = 1.0f;
static float flow_multiplier = 1.0f;

static void commandMove(void)
{
  bool_t axis_involved = FALSE;
  bool_t extruder_involved = FALSE;
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    if (isnan(curr_command->axes_value[i]))
      continue;
    axis_involved = TRUE;
    if (mode.distance == DISTANCEMODE_Relative)
      commanded.axes[i] += curr_command->axes_value[i];
    else
      commanded.axes[i] = curr_command->axes_value[i];
  }
  if (!isnan(curr_command->e_value))
  {
    extruder_involved = true;
    if (mode.extruder_distance == DISTANCEMODE_Relative)
      commanded.extruders[mode.tool] += curr_command->e_value;
    else
      commanded.extruders[mode.tool] = curr_command->e_value;
  }

  if (axis_involved || extruder_involved)
    plannerAddAxisPoint(
        &commanded,
        axis_involved ? mode.feedrate * feedrate_multiplier : mode.feedrate,
        flow_multiplier);
}

static void commandSetPosition(void)
{
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    if (!isnan(curr_command->axes_value[i]))
        commanded.axes[i] = curr_command->axes_value[i];
  }
  if (!isnan(curr_command->e_value))
  {
    commanded.extruders[mode.tool] = curr_command->e_value;
  }
  plannerSetPosition(&commanded);
}

static void printerSyncCommanded(void)
{
  plannerSyncCurrentPosition();
  commanded = stepperGetCurrentPosition();
}
