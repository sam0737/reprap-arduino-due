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

#define sq(x) ((x)*(x))

static PlannerOutputBlock main_buffer[BLOCK_BUFFER_SIZE];
PlannerQueue queueMain;

static float traj_max_feedrate;
static PlannerPhysicalPosition current_physical;
static PlannerVirtualPosition current_virtual;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void plannerAddAxisPointCore(
    const PlannerVirtualPosition *target_virtual,
    float distance, float extrusion_distance, float flow_multiplier,
    float duration);

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * Functions are not reentrant, and should only be invoked
 * from the printer thread
 */

void plannerInit(void)
{
  plannerQueueInit(&queueMain, main_buffer, BLOCK_BUFFER_SIZE);
  traj_max_feedrate = machine.kinematics.traj_max_feedrate(machine) * 60;
}

void plannerSyncCurrentPosition(void)
{
  RadJointsState state = stepperGetJointsState();
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++)
    current_physical.joints[i] = state.joints[i].pos;

  machine.kinematics.forward_kinematics(&current_physical, &current_virtual);
}

void plannerAddAxisPoint(
    const PlannerVirtualPosition *target_virtual,
    float feedrate,
    float flow_multiplier)
{
  (void) feedrate;
  PlannerVirtualPosition delta;
  uint8_t i;
  float distance = 0, extrusion_distance = 0;

  for (i = 0; i < RAD_NUMBER_AXES; i++) {
    delta.axes[i] = target_virtual->axes[i] - current_virtual.axes[i];
    distance += sq(delta.axes[i]);
  }
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    delta.extruders[i] = target_virtual->extruders[i] - current_virtual.extruders[i];
    extrusion_distance += sq(delta.extruders[i]);
  }
  distance = sqrt(distance);
  extrusion_distance = sqrt(extrusion_distance);

  float duration;
  int segments = 1;
  if (distance > 0) {
    duration = 60 * distance / fmin(traj_max_feedrate, feedrate);
    segments = (int)(DELTA_SEGMENTS_PER_SECOND * duration);
    if (segments < 1)
      segments = 1;
  } else {
    /* Pure extrusion case -
     * Assume always have linear mapping between extruder and stepper,
     * so always 1 segment
     */
    duration = 60 * extrusion_distance / feedrate;
  }
  // segments = 1; // TODO: Remove this after debug

  duration /= segments;
  if (segments > 1)
  {
    distance /= segments;
    extrusion_distance /= segments;
    int j = 1;
    for (; j <= segments; j++) {
      PlannerVirtualPosition real_target_virtual;
      float fraction = (float)j / (float)segments;
      for (i = 0; i < RAD_NUMBER_AXES; i++) {
        real_target_virtual.axes[i] = current_virtual.axes[i] + delta.axes[i] * fraction;
      }
      for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
        real_target_virtual.extruders[i] = current_virtual.extruders[i] + delta.extruders[i] * fraction;
      }
      plannerAddAxisPointCore(&real_target_virtual, distance, extrusion_distance, flow_multiplier, duration);
      if (j % (BLOCK_BUFFER_SIZE / 4) == 0) {
        plannerMainQueueRecalculate();
      }
    }
    if (j % (BLOCK_BUFFER_SIZE / 4) != 0) {
      plannerMainQueueRecalculate();
    }
  } else {
    plannerAddAxisPointCore(target_virtual, distance, extrusion_distance, flow_multiplier, duration);
    plannerMainQueueRecalculate();
  }
  current_virtual = *target_virtual;
}

static void plannerAddAxisPointCore(
    const PlannerVirtualPosition *target_virtual,
    float distance, float extrusion_distance, float flow_multiplier,
    float duration)
{
  PlannerPhysicalPosition delta;
  PlannerPhysicalPosition target_physical;
  machine.kinematics.inverse_kinematics(target_virtual, &target_physical);

  float acceleration = 1000000;

  // TODO: PREVENT_DANGEROUS_EXTRUDE

  // Deduce joints speed and speed limit
  if (distance > 0)
  {
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
      RadJoint* jt = &machine.kinematics.joints[i];
      float d = fabs(delta.joints[i] = target_physical.joints[i] - current_physical.joints[i]);
      float fraction = d / distance;
      if (d > duration * jt->max_speed)
        duration = d / jt->max_speed;
      if (fraction > 0 && acceleration * fraction > jt->max_acceleration)
        acceleration = jt->max_acceleration / fraction;
    }
  } else {
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
      delta.joints[i] = 0;
    }
  }

  // Deduce extruders speed and speed limit
  if (extrusion_distance > 0)
  {
    for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
      RadExtruder* ex = &machine.extruder.devices[i];
      float d = fabs(delta.extruders[i] =
          (target_physical.extruders[i] - current_physical.extruders[i]) * flow_multiplier);
      float fraction =  d / extrusion_distance;
      if (target_physical.extruders[i] > current_physical.extruders[i]) {
        if (d > duration * ex->max_speed)
          duration = d / ex->max_speed;
        if (fraction > 0 && acceleration * fraction > ex->max_acceleration)
          acceleration = ex->max_acceleration / fraction;
      } else {
        if (d > duration * ex->max_retract_speed)
          duration = d / ex->max_retract_speed;
        if (fraction > 0 && acceleration * fraction > ex->max_retract_acceleration)
          acceleration = ex->max_retract_acceleration / fraction;
      }
    }
  } else {
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
      delta.extruders[i] = 0;
    }
  }

  if (distance == 0)
    distance = extrusion_distance;
  if (distance == 0)
    return;

  PlannerOutputBlock* block = plannerMainQueueReserveBlock();
  block->mode = BLOCK_Positional;
  block->p.target = current_physical = target_physical;
  block->p.delta = delta;
  block->p.distance = distance;
  block->p.duration = duration;
  block->p.exit_speed = 0;
  block->p.nominal_speed = distance / duration;
  block->p.acc = acceleration;
  /* Law: v^2 - u^2 = 2as, where
   *      v is target velocity,
   *      u is initial velocity,
   *      a is acceleration,
   *      s is distance.
   */
  block->p.is_nominal_length = sq(block->p.nominal_speed) <= 2 * acceleration * distance;
  block->p.is_max_exit_speed_valid = FALSE;
  block->p.is_profile_valid = FALSE;
  plannerMainQueueAddBlock();

  // TODO: Machine boundary
  // TODO: Min Feedrate
  // TODO: Drop segment
  // TODO: Slow down on empty queue
}

void plannerSetJointVelocity(const PlannerJointMovement *velocity)
{
  RadExtruder* ex;
  RadJoint* jt;
  uint8_t i;
  float speed_factor = 1;

  PlannerOutputBlock* block = plannerMainQueueReserveBlock();
  block->mode = BLOCK_Velocity;
  block->stop_on_limit_changes = velocity->stop_on_limit_changes;

  // Deduce joints speed and speed limit
  for (i = 0; i < RAD_NUMBER_JOINTS; i++) {
    jt = &machine.kinematics.joints[i];
    if (isnan(velocity->joints[i]))
    {
      block->v.joints[i].sv = velocity->joints[i];
    } else if (velocity->rapid)
    {
      block->v.joints[i].sv =
          (velocity->joints[i] >= 0 ? 1 : -1) *
            fmin(fabs(velocity->joints[i]), jt->max_speed);
    } else
    {
      if (fabs(velocity->joints[i]) > jt->max_speed) {
        speed_factor = fmin(speed_factor, jt->max_speed / fabs(velocity->joints[i]));
      }
      block->v.joints[i].sv = velocity->joints[i];
    }
    block->v.joints[i].acc = jt->max_acceleration;
  }

  // Deduce extruders speed and speed limit
  for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    ex = &machine.extruder.devices[i];
    if (isnan(velocity->extruders[i]))
    {
      block->v.extruders[i].sv = velocity->extruders[i];
    } else if (velocity->rapid)
    {
      block->v.extruders[i].sv =
          velocity->extruders[i] >= 0 ?
              fmin(velocity->extruders[i], ex->max_speed) :
              fmax(velocity->extruders[i], -ex->max_retract_speed);
    } else
    {
      if (velocity->extruders[i] >= 0) {
        if (velocity->extruders[i] > ex->max_speed) {
          speed_factor = fmin(speed_factor, ex->max_speed / velocity->extruders[i]);
        }
      } else {
        if (velocity->extruders[i] < ex->max_retract_speed) {
          speed_factor = fmin(speed_factor, ex->max_retract_speed / -velocity->extruders[i]);
        }
      }
      block->v.extruders[i].sv = velocity->extruders[i];
    }
    block->v.extruders[i].acc =
      velocity->extruders[i] >= 0 ?
        ex->max_acceleration :
        ex->max_retract_acceleration;
  }

  // Scale down if in non-rapid mode
  if (!velocity->rapid) {
    for (i = 0; i < RAD_NUMBER_JOINTS; i++) {
      if (isnan(block->v.joints[i].sv))
        continue;
      block->v.joints[i].sv *= speed_factor;
    }
    for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
      if (isnan(block->v.extruders[i].sv))
        continue;
      block->v.extruders[i].sv *= speed_factor;
    }
  }

  plannerMainQueueAddBlock();
  plannerMainQueueCommit();
}

void plannerSetPosition(const PlannerVirtualPosition *virtual)
{
  current_virtual = *virtual;
  machine.kinematics.inverse_kinematics(virtual, &current_physical);

  PlannerOutputBlock* block = plannerMainQueueReserveBlock();
  block->mode = BLOCK_Reset;
  block->p.target = current_physical;
  plannerMainQueueAddBlock();
  plannerMainQueueCommit();
}

void plannerEstop(void)
{
  PlannerOutputBlock* block = plannerMainQueueReserveBlock();
  block->mode = BLOCK_Estop;
  plannerMainQueueInterruptCommit(block);
}

void plannerEstopClear(void)
{
  PlannerOutputBlock* block = plannerMainQueueReserveBlock();
  block->mode = BLOCK_Estop_Clear;
  plannerMainQueueAddBlock();
  plannerMainQueueCommit();
}
/** @} */
