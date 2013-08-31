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
  bool_t rapid;
  bool_t stop_on_limit_changes;
  float joints[RAD_NUMBER_AXES];
  float extruders[RAD_NUMBER_EXTRUDERS];
} PlannerJointMovement;

typedef enum {
  BLOCK_Preemptive_Mask = 0x10,

  BLOCK_Velocity = 1 | 0x10,
  BLOCK_Planned = 2,
  BLOCK_Stop = 3 | 0x10,

  BLOCK_Estop = 4 | 0x10,
  BLOCK_Estop_Clear = 5,
} PlannerOutputBlockMode;

typedef struct {
  uint32_t era;
  PlannerOutputBlockMode mode;
  bool_t stop_on_limit_changes;
  struct {
    union {
      struct {
        float sv;
        bool_t is_stop_signalled;
      } velocity;
    } data;
    float acceleration;
  } joints[RAD_NUMBER_JOINTS];
  struct {
    union {
      struct {
        float sv;
      } velocity;
    } data;
    float acceleration;
  } extruders[RAD_NUMBER_EXTRUDERS];
} PlannerOutputBlock;

/*===========================================================================*/
/* Macros.                                                                   */
/*===========================================================================*/

#define plannerFetchBlockI(blockp) (chMBFetchI(&block_mbox, (msg_t*)&blockp))

#define plannerFreeBlockI(blockp) do {                                    \
  chPoolFreeI(&block_pool, blockp);                                       \
  chSemSignalI(&block_pool_sem);                                             \
} while(0)

#define plannerPeekBlockI(blockp) do {                                    \
  blockp = chMBGetUsedCountI(&block_mbox) > 0 ?                           \
    (PlannerOutputBlock*)chMBPeekI(&block_mbox) : NULL;                   \
} while (0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern Semaphore block_pool_sem;
extern MemoryPool block_pool;
extern Mailbox block_mbox;

#ifdef __cplusplus
extern "C" {
#endif
void plannerInit(void);
void plannerAddAxisPoint(const PlannerAxisMovement *point);
void plannerSetJointVelocity(const PlannerJointMovement *velocity);
void plannerEstop(void);
void plannerEstopClear(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_PLANNER_H_ */

/** @} */
