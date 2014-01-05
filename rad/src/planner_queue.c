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
 * @file    planner_queue.c
 * @brief   Trajectory planner queue
 *
 * @addtogroup PLANNER_QUEUE
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "planner_queue.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void plannerQueueInit(PlannerQueue* queue, PlannerOutputBlock* buffer, size_t size)
{
  queue->q_counter = 0;
  queue->q_pending = 0;
  /*
   * Reduce the size such that rd_ptr will never be overridden during recalculation
   */
  chSemInit(&queue->q_sem, BLOCK_BUFFER_SIZE - 3);
  queue->q_buffer = queue->q_wrptr = queue->q_rdptr = buffer;
  queue->q_top = queue->q_buffer + size;
}

bool_t plannerQueueFetchBlockI(PlannerQueue* queue, PlannerOutputBlock* block)
{
  if (queue->q_counter == 0)
    return FALSE;

  *block = *queue->q_rdptr;
  block->busy = TRUE;

  if (block->mode == BLOCK_Positional) {
    queue->last_exit_speed = block->p.exit_speed;
  } else {
    queue->last_exit_speed = 0;
  }
  queue->q_counter--;

  if (++queue->q_rdptr >= queue->q_top)
    queue->q_rdptr = queue->q_buffer;

  chSemSignalI(&queue->q_sem);
  return TRUE;
}

PlannerOutputBlock* plannerQueueReserveBlock(PlannerQueue* queue)
{
  chSemWait(&queue->q_sem);
  PlannerOutputBlock* block = queue->q_wrptr;
  block->busy = FALSE;
  block->p.is_profile_valid = FALSE;
  return block;
}

void plannerQueueAddBlock(PlannerQueue* queue)
{
  chSysLock();
  queue->q_pending++;
  if (++queue->q_wrptr >= queue->q_top)
    queue->q_wrptr = queue->q_buffer;
  chSysUnlock();
}

void plannerQueueCommit(PlannerQueue* queue)
{
  chSysLock();
  queue->q_counter += queue->q_pending;
  queue->q_pending = 0;
  chSysUnlock();
}

static void recalculateMaxExitSpeed(PlannerOutputBlock *current, PlannerOutputBlock *next)
{
  current->p.is_max_exit_speed_valid = TRUE;
  float duration = current->p.duration;

  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    if (fabs(current->p.delta.joints[i]) < 0.000001) continue;
    float v = next->p.delta.joints[i] / next->p.duration;
    if (fabs(v) < 0.000001) {
      current->p.max_exit_speed = 0;
      return;
    }
    float d = current->p.delta.joints[i] / v;
    if (d < 0) {
      current->p.max_exit_speed = 0;
      return;
    }
    if (d > duration) duration = d;
  }

  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    if (fabs(current->p.delta.extruders[i]) < 0.000001) continue;
    float v = next->p.delta.extruders[i] / next->p.duration;
    if (fabs(v) < 0.000001) {
      current->p.max_exit_speed = 0;
      return;
    }
    float d = current->p.delta.extruders[i] / v;
    if (d < 0) {
      current->p.max_exit_speed = 0;
      return;
    }
    if (d > duration) duration = d;
  }

  current->p.max_exit_speed =
    current->p.nominal_speed *
    current->p.duration / duration;
}

static void ralculateReversePassKernel(PlannerOutputBlock *current, PlannerOutputBlock *next)
{
  if (current->busy || current->mode != BLOCK_Positional)
    return;
  if (next == NULL || next->mode != BLOCK_Positional) {
    current->p.is_profile_valid = FALSE;
    current->p.exit_speed = 0;
    return;
  }

  if (!current->p.is_max_exit_speed_valid)
    recalculateMaxExitSpeed(current, next);

  /*
   * The queue has long enough braking distance such that exit_speed can reach max_exit_speed.
   * Assumption: The queue is never cut short
   */
  if (current->p.exit_speed == current->p.max_exit_speed)
    return;

  current->p.is_profile_valid = FALSE;
  if (next->p.is_nominal_length) {
    /*
     * If the next block can accelerate/decelerate to 0 from any speed...
     */
    current->p.exit_speed = current->p.max_exit_speed;
  } else
  {
    /*
     * If not, calculate the speed such that the next block could decelerate to its exit speed
     */
    current->p.exit_speed =
        fmin(
            current->p.max_exit_speed,
            sqrt(next->p.exit_speed * next->p.exit_speed + 2 * next->p.acc * next->p.distance)
        );
  }
}

static void ralculateReversePass(PlannerQueue *queue, PlannerOutputBlock *head, PlannerOutputBlock *tail)
{
  PlannerOutputBlock *next = NULL;
  do
  {
    if (tail == queue->q_buffer)
      tail = queue->q_top;
    --tail;
    ralculateReversePassKernel(tail, next);
    next = tail;
  } while (head != tail);
}

static void ralculateForwardPassKernel(float last_exit_speed, PlannerOutputBlock *current)
{
  if (current->p.is_nominal_length || !current->p.is_max_exit_speed_valid)
    return;

  if (last_exit_speed >= current->p.exit_speed)
    return;

  float new_exit_speed =
      fmin(
          current->p.exit_speed,
          sqrt(last_exit_speed * last_exit_speed + 2 * current->p.acc * current->p.distance)
      );

  if (new_exit_speed == current->p.exit_speed)
    return;

  current->p.exit_speed = new_exit_speed;
  current->p.is_profile_valid = FALSE;
}

static void calculateTrapezoid(float last_exit_speed, PlannerOutputBlock *current)
{
  current->p.is_profile_valid = TRUE;
  if (current->p.acc == 0)
  {
    RAD_DEBUG_PRINTF("  T 0\n");
    current->p.accelerate_until = 0;
    current->p.decelerate_after = 0;
    return;
  }

  float accel_distance =
      (current->p.nominal_speed * current->p.nominal_speed - last_exit_speed * last_exit_speed) /
      2 / current->p.acc;
  float decel_distance =
      (current->p.nominal_speed * current->p.nominal_speed - current->p.exit_speed * current->p.exit_speed) /
      2 / current->p.acc;

  RAD_DEBUG_PRINTF("T %f %f\n", accel_distance, decel_distance);
  float plateau = current->p.distance- accel_distance - decel_distance;
  if (plateau < 0)
  {
    accel_distance =
        (
            2 * current->p.acc * current->p.distance -
            last_exit_speed * last_exit_speed +
            current->p.exit_speed * current->p.exit_speed
        ) / 4 / current->p.acc;
    if (accel_distance < 0) accel_distance = 0;
    plateau = 0;
  }

  current->p.accelerate_until = accel_distance;
  current->p.decelerate_after = accel_distance + plateau;
}

static void ralculateForwardPass(PlannerQueue *queue, PlannerOutputBlock *head, PlannerOutputBlock *tail)
{
  PlannerOutputBlock *prev = NULL;
  do
  {
    if (!head->busy && head->mode == BLOCK_Positional)
    {
      float last_exit_speed =
          (prev == NULL || prev->busy || prev->mode != BLOCK_Positional) ?
              queue->last_exit_speed : prev->p.exit_speed;
      ralculateForwardPassKernel(last_exit_speed, head);
      if (!head->p.is_profile_valid)
        calculateTrapezoid(last_exit_speed, head);
    }

    prev = head;
    if (++head == queue->q_top)
      head = queue->q_buffer;
  } while (head != tail);
}

void plannerQueueRecalculate(PlannerQueue *queue)
{
  PlannerOutputBlock* tail;
  PlannerOutputBlock* head;
  chSysLock();
  if (queue->q_pending == 0 && queue->q_counter == 0) {
    chSysUnlock();
    return;
  }
  tail = queue->q_wrptr;
  head = queue->q_rdptr;
  chSysUnlock();

  ralculateReversePass(queue, head, tail);
  ralculateForwardPass(queue, head, tail);

  plannerQueueCommit(queue);
}

/** @} */
