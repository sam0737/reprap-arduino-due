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
 * @file    planner.h
 * @brief   Trajectory planner header
 *
 * @addtogroup PLANNER
 * @{
 */

#ifndef _RAD_PLANNER_H_
#define _RAD_PLANNER_H_

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

#define BLOCK_BUFFER_SIZE 200
#define DELTA_SEGMENTS_PER_SECOND 200

typedef struct {
  float axes[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} PlannerVirtualPosition;

typedef struct {
  float joints[RAD_NUMBER_JOINTS];
  float extruders[RAD_NUMBER_EXTRUDERS];
} PlannerPhysicalPosition;

typedef struct {
  bool_t rapid;
  bool_t stop_on_limit_changes;
  float joints[RAD_NUMBER_JOINTS];
  float extruders[RAD_NUMBER_EXTRUDERS];
} PlannerJointMovement;

/*===========================================================================*/
/* Macros.                                                                   */
/*===========================================================================*/

#define plannerMainQueueFetchBlockI(block_p, current_mode) plannerQueueFetchBlockI(&queueMain, block_p, current_mode)
#define plannerMainQueueReserveBlock() plannerQueueReserveBlock(&queueMain)
#define plannerMainQueueAddBlock() plannerQueueAddBlock(&queueMain)
#define plannerMainQueueCommit() plannerQueueCommit(&queueMain)
#define plannerMainQueueInterruptCommit(block_p) plannerQueueInterruptCommit(&queueMain, block_p)
#define plannerMainQueueRecalculate() plannerQueueRecalculate(&queueMain)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#include "planner_queue.h"
extern PlannerQueue queueMain;

#ifdef __cplusplus
extern "C" {
#endif
  void plannerInit(void);
  PlannerVirtualPosition plannerGetCurrentPosition(void);
  void plannerAddAxisPoint(const PlannerVirtualPosition *target, float feedrate);
  void plannerSetJointVelocity(const PlannerJointMovement *velocity);
  void plannerSetPosition(const PlannerVirtualPosition *virtual);
  void plannerEstop(void);
  void plannerEstopClear(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_PLANNER_H_ */

/** @} */
