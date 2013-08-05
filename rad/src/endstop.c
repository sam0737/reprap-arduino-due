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
 * @file    endstop.c
 * @brief   Endstop management
 *
 * @addtogroup ENDSTOP
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "endstop.h"


/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waEndstop, 128);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void checkEndstop(void)
{
  int8_t i;
  int8_t j;
  RadJoint* joint;
  for (i = 0; i < machine.kinematics.joint_count; i++)
  {
    joint = &machine.kinematics.joints[i];
    j = joint->min_endstop_id;
    if (j >= 0 &&
        palReadPin(radboard.endstop.channels[j].pin, machine.endstop_config.configs[j].active_low)) {
      joint->state.limit_state = LIMIT_MinHit;
      continue;
    }
    j = joint->max_endstop_id;
    if (j >= 0 &&
        palReadPin(radboard.endstop.channels[j].pin, machine.endstop_config.configs[j].active_low)) {
      joint->state.limit_state = LIMIT_MaxHit;
      continue;
    }
    joint->state.limit_state = LIMIT_Normal;
  }
}

static msg_t threadEndstop(void *arg) {
  (void)arg;
  chRegSetThreadName("endstop");

  while (TRUE) {
    chThdSleepMilliseconds(1);
    checkEndstop();
  }
  return 0;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void endstopInit(void)
{
  uint8_t i;
  for (i = 0; i < radboard.endstop.count; i++) {
    palSetPinMode(radboard.endstop.channels[i].pin, PAL_MODE_INPUT_PULLUP);
  }

  chThdCreateStatic(waEndstop, sizeof(waEndstop), NORMALPRIO + 5, threadEndstop, NULL);
}

/** @} */
