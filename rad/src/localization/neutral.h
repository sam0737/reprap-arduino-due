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
  "S0-Ready"
#endif

#ifndef L_UI_STATUS_NOT_HOMED
#define L_UI_STATUS_NOT_HOMED \
  "S1-Printer is not homed"
#endif

#ifndef L_DEBUG_STOPPED_BY_SHELL
#define L_DEBUG_STOPPED_BY_SHELL \
  "S2-Stopped by shell"
#endif

#ifndef L_HOMING_INCORRECT_INIT_STATE
#define L_HOMING_INCORRECT_INIT_STATE \
  "H1-Incorrect initial limit switch state for homing"
#endif

#ifndef L_HOMING_TRAVEL_LIMIT
#define L_HOMING_TRAVEL_LIMIT \
  "H2-Joint has traveled too far without triggering any limit switch"
#endif

#ifndef L_HOMING_INCORRECT_LIMIT_HIT
#define L_HOMING_INCORRECT_LIMIT_HIT \
  "H3-Incorrect limit switch is hit when homing"
#endif

#ifndef L_TEMPERATURE_AUTOTUNE_OVERHEATED
#define L_TEMPERATURE_AUTOTUNE_OVERHEATED \
  "T1-Autotune overheated"
#endif

#ifndef L_TEMPERATURE_AUTOTUNE_TIMEOUT
#define L_TEMPERATURE_AUTOTUNE_TIMEOUT \
  "T2-Autotune timeout"
#endif

#ifndef L_TEMPERATURE_AUTOTUNE_IN_ESTOP
#define L_TEMPERATURE_AUTOTUNE_IN_ESTOP \
  "T3-Autotune aborted by ESTOP"
#endif
