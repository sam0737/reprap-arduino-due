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
 * @file    planner_queue.h
 * @brief   Trajectory planner queue header
 *
 * @addtogroup PLANNER_QUEUE
 * @{
 */

#ifndef _RAD_PLANNER_QUEUE_H_
#define _RAD_PLANNER_QUEUE_H_

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

typedef enum {
  BLOCK_Idle = 0,

  BLOCK_Velocity = 1,
  BLOCK_Positional = 2,
  BLOCK_Command = 3,

  BLOCK_Estop = 4,
  BLOCK_Estop_Clear = 5,
  BLOCK_Reset = 6
} PlannerOutputBlockMode;

typedef struct {
  struct {
    float sv;
    float acc;
    bool_t is_stop_signalled;
  } joints[RAD_NUMBER_JOINTS];
  struct {
    float sv;
    float acc;
  } extruders[RAD_NUMBER_EXTRUDERS];
} PlannerOutputBlockSectionV;

typedef struct {
  bool_t is_max_exit_speed_valid;
  bool_t is_nominal_length;

  bool_t is_profile_valid;
  PlannerPhysicalPosition target;
  PlannerPhysicalPosition delta;
  struct {
    uint32_t joint_dir_mask;
    uint32_t extruder_dir_mask;
    uint32_t joints[RAD_NUMBER_JOINTS];
    uint32_t extruders[RAD_NUMBER_EXTRUDERS];
    uint32_t total;
  } step;
  float duration;
  float acc;
  float distance;

  float nominal_speed;
  float exit_speed;

  float max_exit_speed;
  float decelerate_after;
} PlannerOutputBlockSectionP;

typedef struct {
  PlannerOutputBlockMode mode;
  bool_t busy;
  bool_t stop_on_limit_changes;
  union {
    PlannerOutputBlockSectionV v;
    PlannerOutputBlockSectionP p;
  };
} PlannerOutputBlock;

typedef struct {
  Semaphore             q_sem;
  size_t                q_pending; /**< @brief Uncommitted block counter.  */
  size_t                q_counter; /**< @brief Resources counter.          */
  PlannerOutputBlock*   q_buffer;  /**< @brief Pointer to the queue buffer.*/
  PlannerOutputBlock*   q_top;     /**< @brief Pointer to the first location
                                                after the buffer.          */
  PlannerOutputBlock*   q_wrptr;   /**< @brief Write pointer.              */
  PlannerOutputBlock*   q_rdptr;   /**< @brief Read pointer.               */

  float                 last_exit_speed;
} PlannerQueue;

/*===========================================================================*/
/* Macros.                                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void plannerQueueInit(PlannerQueue* queue, PlannerOutputBlock* buffer, size_t size);
  size_t plannerQueueGetLength(PlannerQueue* queue);
  bool_t plannerQueueFetchBlockI(PlannerQueue* queue, PlannerOutputBlock* block, PlannerOutputBlockMode current_mode);
  PlannerOutputBlock* plannerQueueReserveBlock(PlannerQueue* queue);
  void plannerQueueAddBlock(PlannerQueue* queue);
  void plannerQueueCommit(PlannerQueue* queue);
  void plannerQueueInterruptCommit(PlannerQueue* queue, PlannerOutputBlock* block);
  void plannerQueueRecalculate(PlannerQueue *queue);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_PLANNER_H_ */

/** @} */
