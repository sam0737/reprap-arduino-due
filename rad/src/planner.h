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
 * @addtogroup ENDSTOP
 * @{
 */

#ifndef _RAD_PLANNER_H_
#define _RAD_PLANNER_H_

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

#define BLOCK_BUFFER_SIZE 256

typedef struct {
  float axes[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
  float linear_feedrate;
} PlannerAxisMovement;

typedef struct {
  uint8_t rapid:1;
  float joints[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} PlannerJointMovement;

typedef struct {
  uint32_t era;
  bool_t velocity_mode:1;
  union {
    struct {
      float joints[RAD_NUMBER_JOINTS];
      float extruders[RAD_NUMBER_EXTRUDERS];
    } velocity;
  } data;
  struct {
    uint32_t joints[RAD_NUMBER_JOINTS];
    uint32_t extruders[RAD_NUMBER_EXTRUDERS];
  } acceleration;
} PlannerOutputBlock;


/*===========================================================================*/
/* Macros.                                                                   */
/*===========================================================================*/

#define plannerFetchBlockI(blockp) (chMBFetchI(&block_mbox, (msg_t*)&blockp))

#define plannerFreeBlockI(blockp) (chPoolFreeI(&block_pool, &blockp))

#define plannerPeekBlockI(blockp) do {                                    \
  blockp = chMBGetUsedCountI(&block_mbox) > 0 ?                           \
    (PlannerOutputBlock*)chMBPeekI(&block_mbox) : NULL;                   \
} while (0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern MemoryPool block_pool;
extern Mailbox block_mbox;

#ifdef __cplusplus
extern "C" {
#endif
void plannerInit(void);
void plannerAddAxisPoint(PlannerAxisMovement *point);
void plannerSetJointVelocity(PlannerJointMovement *velocity);
void plannerSetAxisVelocity(PlannerAxisMovement *velocity);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_PLANNER_H_ */

/** @} */
