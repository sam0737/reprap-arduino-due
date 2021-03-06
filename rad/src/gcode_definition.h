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

#ifndef _GCODE_DEFINITION_H_
#define _GCODE_DEFINITION_H_

#include "rad.h"

typedef enum {
  RAPIDMODE_Rapid = 1,
  RAPIDMODE_Feed = 2,
} RapidMode;

typedef enum {
  DISTANCEMODE_Absolute = 1,
  DISTANCEMODE_Relative = 2
} DistanceMode;

typedef enum {
  UNITMODE_Millimeter = 1,
  UNITMODE_Inch = 2
} UnitMode;

typedef enum {
  POWERMODE_Off = 1,
  POWERMODE_Sleep = 2,
  POWERMODE_On = 3,
  POWERMODE_Idle = 4
} PowerMode;

typedef struct {
  RapidMode rapid;
  DistanceMode distance;
  DistanceMode extruder_distance;
  UnitMode unit;
  int8_t tool;
  float feedrate;
} PrinterMode;

typedef enum {
  WAITMODE_CurrentTool = 1,
  WAITMODE_HeatedBed = 2,
  WAITMODE_All = 3
} WaitMode;

typedef enum {
  COMMANDTYPE_None = 0,
  COMMANDTYPE_Action = 0x01,
  COMMANDTYPE_SyncAction = (0x01 | 0x02),
  COMMANDTYPE_CanHaveAxisWords = 0x04,
  COMMANDTYPE_Movement = (0x04 | 0x08),
  COMMANDTYPE_TimeStart = 0x10,
  COMMANDTYPE_UnknownCode = 0x80,
  COMMANDTYPE_PrinterInterrupt = 0x1000,
  COMMANDTYPE_PrinterResume = 0x2000
} CommandType;

/** Printer command **/
typedef struct {
  PrinterMode printer;
  PowerMode power;
  WaitMode wait;

  CommandType type;

  /** G or M code **/
  uint16_t code;

  /** Line number **/
  int32_t line;

  /** Parameter: Tool **/
  int8_t t_value;

  /** Parameter: R **/
  float r_value;
  /** Parameter: S **/
  float s_value;
  /** Parameter: P **/
  int32_t p_value;

  /** Motion parameter: Extruder **/
  float e_value;
  /** Motion parameter: Axes **/
  float axes_value[RAD_NUMBER_AXES];

  Mailbox* ack_mbox;
  EventSource* ack_evt;
} PrinterCommand;

#endif
