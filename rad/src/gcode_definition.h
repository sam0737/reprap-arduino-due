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
  COMMANDTYPE_Action = 1,
  COMMANDTYPE_CanHaveAxisWords = 0x02,
  COMMANDTYPE_Movement = (0x02 | 0x04),
  COMMANDTYPE_UnknownMcode = 0x08,
  COMMANDTYPE_UnknownGcode = 0x10
} CommandType;

typedef struct {
  PrinterMode printer;
  PowerMode power;
  WaitMode wait;

  CommandType type;
  uint16_t code;

  int32_t line;

  float r_value;
  float s_value;
  int32_t p_value;

  int8_t t_value;

  float e_value;
  float axes_value[RAD_NUMBER_AXES];

  Mailbox* ack_mbox;
  EventSource* ack_evt;
} PrinterCommand;

#endif
