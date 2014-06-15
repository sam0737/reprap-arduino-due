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

/**
 * @file    localization/neutral.h
 * @brief   Neutral language definition
 *
 */

#ifndef L_UI_STATUS_READY
#define L_UI_STATUS_READY \
  "S00-Ready"
#endif

#ifndef L_UI_STATUS_PRINTING
#define L_UI_STATUS_PRINTING \
  "S01-Printing"
#endif

#ifndef L_UI_STATUS_INTERRUPTED
#define L_UI_STATUS_INTERRUPTED \
  "S05-Paused"
#endif

#ifndef L_UI_STATUS_INTERRUPTING
#define L_UI_STATUS_INTERRUPTING \
  "S03-Pausing"
#endif

#ifndef L_PRINTER_STATUS_HEATING
#define L_PRINTER_STATUS_HEATING \
  "S02-Heating up"
#endif

#ifndef L_PRINTER_STATUS_NOT_HOMED
#define L_PRINTER_STATUS_NOT_HOMED \
  "E00-Printer is not homed"
#endif

#ifndef L_DEBUG_STOPPED_BY_SHELL
#define L_DEBUG_STOPPED_BY_SHELL \
  "E01-Stopped by shell"
#endif

#ifndef L_DEBUG_STOPPED_BY_MENU
#define L_DEBUG_STOPPED_BY_MENU \
  "E02-Stopped by menu"
#endif

#ifndef L_HOMING_INCORRECT_INIT_STATE
#define L_HOMING_INCORRECT_INIT_STATE \
  "E10-Incorrect initial limit switch state for homing"
#endif

#ifndef L_HOMING_TRAVEL_LIMIT
#define L_HOMING_TRAVEL_LIMIT \
  "E11-Joint has traveled too far without triggering any limit switch"
#endif

#ifndef L_HOMING_INCORRECT_LIMIT_HIT
#define L_HOMING_INCORRECT_LIMIT_HIT \
  "E12-Incorrect limit switch is hit when homing"
#endif

#ifndef L_TEMPERATURE_AUTOTUNE_OVERHEATED
#define L_TEMPERATURE_AUTOTUNE_OVERHEATED \
  "E20-Autotune overheated"
#endif

#ifndef L_TEMPERATURE_AUTOTUNE_TIMEOUT
#define L_TEMPERATURE_AUTOTUNE_TIMEOUT \
  "E21-Autotune timeout"
#endif

#ifndef L_UI_BACK
#define L_UI_BACK \
  "Back"
#endif

#ifndef L_UI_MAINMENU_ESTOP_CLEAR
#define L_UI_MAINMENU_ESTOP_CLEAR \
  "Reset Estop"
#endif

#ifndef L_UI_MAINMENU_RESUME_PRINTING
#define L_UI_MAINMENU_RESUME_PRINTING \
  "Resume"
#endif

#ifndef L_UI_MAINMENU_TUNING
#define L_UI_MAINMENU_TUNING \
  "Tuning"
#endif

#ifndef L_UI_MAINMENU_INTERRUPT_NOW
#define L_UI_MAINMENU_INTERRUPT_NOW \
  "Pause"
#endif

#ifndef L_UI_MAINMENU_PRINT
#define L_UI_MAINMENU_PRINT \
  "Print File"
#endif

#ifndef L_UI_MAINMENU_STORAGE_UMS
#define L_UI_MAINMENU_STORAGE_UMS \
  "USB Storage On"
#endif

#ifndef L_UI_MAINMENU_STORAGE_LOCAL
#define L_UI_MAINMENU_STORAGE_LOCAL \
  "USB Storage Off"
#endif

#ifndef L_UI_MAINMENU_PREPARE
#define L_UI_MAINMENU_PREPARE \
  "Prepare"
#endif

#ifndef L_UI_MAINMENU_INFO
#define L_UI_MAINMENU_INFO \
  "Info"
#endif

#ifndef L_UI_MAINMENU_ESTOP
#define L_UI_MAINMENU_ESTOP \
  "Emergency Stop"
#endif

#ifndef L_UI_MAINMENU_POWER_OFF
#define L_UI_MAINMENU_POWER_OFF \
  "Power Off"
#endif

#ifndef L_UI_PRINT_UP
#define L_UI_PRINT_UP \
  "Up"
#endif

#ifndef L_UI_PRINT_NO_FILES
#define L_UI_PRINT_NO_FILES \
  "- No Files -"
#endif

#ifndef L_UI_PREPARE_HOMING
#define L_UI_PREPARE_HOMING \
  "Homing"
#endif

#ifndef L_PRINTER_HOST_GCODE_ERROR
#define L_PRINTER_HOST_GCODE_ERROR \
  "E50-Invalid GCode from host"
#endif

#ifndef L_PRINTER_STOPPED_BY_HOST
#define L_PRINTER_STOPPED_BY_HOST \
  "E03-Stopped by host"
#endif
