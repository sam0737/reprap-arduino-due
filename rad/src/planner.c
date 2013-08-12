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
 * @file    planner.c
 * @brief   Trajectory planner
 *
 * @addtogroup PLANNER
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "planner.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static PlannerOutputBlock block_pool_buffer[BLOCK_BUFFER_SIZE + 3];
static msg_t block_mbox_buffer[BLOCK_BUFFER_SIZE];

Semaphore block_pool_sem;
MemoryPool block_pool;
Mailbox block_mbox;

static uint32_t current_era = 0;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static PlannerOutputBlock* planner_new_block(void)
{
  chSemWait(&block_pool_sem);
  PlannerOutputBlock* block = chPoolAlloc(&block_pool);
  block->era = ++current_era;
  return block;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void plannerInit(void)
{
  chPoolInit(&block_pool, sizeof(PlannerOutputBlock), NULL);
  chPoolLoadArray(&block_pool, block_pool_buffer, BLOCK_BUFFER_SIZE + 3);
  chMBInit(&block_mbox, block_mbox_buffer, BLOCK_BUFFER_SIZE);
  chSemInit(&block_pool_sem, BLOCK_BUFFER_SIZE + 3);
}

void plannerAddAxisPoint(PlannerAxisMovement *point)
{

}

void plannerSetJointVelocity(PlannerJointMovement *velocity)
{
  RadExtruder* ex;
  RadJoint* jt;
  uint8_t i;
  float speed_factor = 1;

  PlannerOutputBlock* block = planner_new_block();
  block->mode = BLOCK_Velocity;
  block->stop_on_limit_changes = velocity->stop_on_limit_changes;

  // Deduce joints speed and speed limit
  for (i = 0; i < machine.kinematics.joint_count; i++) {
    jt = &machine.kinematics.joints[i];
    if (velocity->rapid) {
      block->joints[i].data.velocity =
          (velocity->joints[i] >= 0 ? 1 : -1) *
            fmin(fabs(velocity->joints[i]), jt->max_velocity);
    } else {
      if (fabs(velocity->joints[i]) > jt->max_velocity) {
        speed_factor = fmin(speed_factor, jt->max_velocity / fabs(velocity->joints[i]));
      }
      block->joints[i].data.velocity = velocity->joints[i];
    }
    block->joints[i].acceleration = jt->max_acceleration;
  }

  // Deduce extruders speed and speed limit
  for (i = 0; i < machine.extruder.count; i++) {
    ex = &machine.extruder.devices[i];
    if (velocity->rapid) {
      block->extruders[i].data.velocity =
          velocity->extruders[i] >= 0 ?
              fmin(velocity->extruders[i], ex->max_velocity) :
              fmax(velocity->extruders[i], -ex->max_retract_velocity);
    } else {
      if (velocity->extruders[i] >= 0) {
        if (velocity->extruders[i] > ex->max_velocity) {
          speed_factor = fmin(speed_factor, ex->max_velocity / velocity->extruders[i]);
        }
      } else {
        if (velocity->extruders[i] < ex->max_retract_velocity) {
          speed_factor = fmin(speed_factor, ex->max_retract_velocity / -velocity->extruders[i]);
        }
      }
      block->extruders[i].data.velocity = velocity->extruders[i];
    }
    block->extruders[i].acceleration =
      velocity->extruders[i] >= 0 ?
        ex->max_acceleration :
        ex->max_retract_acceleration;
  }

  // Scale down if in non-rapid mode
  if (!velocity->rapid) {
    for (i = 0; i < machine.kinematics.joint_count; i++) {
      block->joints[i].data.velocity *= speed_factor;
    }
    for (i = 0; i < machine.extruder.count; i++) {
      block->extruders[i].data.velocity *= speed_factor;
    }
  }

  // Post to the top of the queue
  chMBPostAhead(&block_mbox, (msg_t) block, TIME_INFINITE);
}

void plannerEstop(void)
{
  PlannerOutputBlock* block = planner_new_block();
  block->mode = BLOCK_Estop;
  chMBPostAhead(&block_mbox, (msg_t) block, TIME_INFINITE);
}

void plannerEstopClear(void)
{
  PlannerOutputBlock* block = planner_new_block();
  block->mode = BLOCK_Estop_Clear;
  chMBPost(&block_mbox, (msg_t) block, TIME_INFINITE);
}
/** @} */
