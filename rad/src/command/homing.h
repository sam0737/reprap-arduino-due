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

typedef enum {
  HOMING_NONE = 0,
  HOMING_SEARCH = 1,
  HOMING_SEARCH_BACKOFF = 2,
  HOMING_LATCH_RELEASE = 3,
  HOMING_LATCH_HIT = 5,
  HOMING_PRE_FINAL = 6,
  HOMING_FINAL = 7,
} HomingStage;

typedef struct {
  HomingStage stage;
  float last_pos;
} HomingState;

static void commandHoming(uint32_t joint_mask)
{
  HomingState state[RAD_NUMBER_JOINTS];
  uint8_t sequence = 0;
  uint8_t count = 0;
  while (1) {
    PlannerJointMovement m;
    for (uint8_t i = 0; i < machine.kinematics.joint_count; i++)
    {
      RadJoint* j = &machine.kinematics.joints[i];
      if (j->home_sequence != sequence ||
          j->home_search_vel == 0 ||
          j->home_latch_vel == 0) continue;

      count++;
      if (joint_mask & (1 << i)) {
        m.joints[i] = j->home_search_vel;
        state[i].last_pos = j->state.pos;
        state[i].stage = HOMING_SEARCH;
      }
    }
    if (count == 0) break;

    m.rapid = 1;
    m.stop_on_limit_changes = TRUE;
    plannerSetJointVelocity(&m);

    while (count > 0)
    {
      chThdSleepMilliseconds(50);
      for (uint8_t i = 0; i < machine.kinematics.joint_count; i++)
      {
        RadJoint* j = &machine.kinematics.joints[i];
        switch (state[i].stage)
        {
        case HOMING_NONE:
          continue;
        case HOMING_SEARCH:
          if (fabs(state[i].last_pos - j->state.pos) >
              (j->max_limit - j->min_limit) * 1.5) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
            return;
          }
          if (!j->state.stopped) continue;
          if (j->state.limit_state == LIMIT_Normal) {
            m.joints[i] = j->home_search_vel;
            plannerSetJointVelocity(&m);
          } else {
            if ((j->home_search_vel < 0 && j->state.limit_state == LIMIT_MinHit) ||
                (j->home_search_vel > 0 && j->state.limit_state == LIMIT_MaxHit))
            {
              state[i].last_pos = j->state.pos;
              if ((j->home_latch_vel > 0 && j->home_latch_vel > 0) ||
                  (j->home_latch_vel < 0 && j->home_latch_vel < 0))
              {
                state[i].stage = HOMING_SEARCH_BACKOFF;
                m.joints[i] = -j->home_search_vel;
              } else {
                state[i].stage = HOMING_LATCH_RELEASE;
                m.joints[i] = j->home_latch_vel;
              }
              plannerSetJointVelocity(&m);
            } else {
              printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
            }
          }
          break;
        case HOMING_SEARCH_BACKOFF:
          if (fabs(state[i].last_pos - j->state.pos) >
              fmin((j->max_limit - j->min_limit) / 20.0, 20)) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
          }
          if (!j->state.stopped) continue;
          if (j->state.limit_state == LIMIT_Normal) {
            state[i].stage = HOMING_LATCH_HIT;
            m.joints[i] = j->home_latch_vel;
            plannerSetJointVelocity(&m);
          } else {
            printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
          }
          break;
        case HOMING_LATCH_RELEASE:
          if (fabs(state[i].last_pos - j->state.pos) >
              fmin((j->max_limit - j->min_limit) / 20.0, 20)) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
          }
          if (!j->state.stopped) continue;
          if (j->state.limit_state == LIMIT_Normal) {
            stepperSetHome(i,
                j->state.limit_step,
                j->state.limit_state == LIMIT_MinHit ? j->min_limit : j->max_limit);
            state[i].stage = HOMING_PRE_FINAL;
          }
          break;
        case HOMING_LATCH_HIT:
          if (fabs(state[i].last_pos - j->state.pos) >
              fmin((j->max_limit - j->min_limit) / 20.0, 20)) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
          }
          if (!j->state.stopped) continue;
          if (j->state.limit_state != LIMIT_Normal) {
            if ((j->home_latch_vel < 0 && j->state.limit_state == LIMIT_MinHit) ||
                (j->home_latch_vel > 0 && j->state.limit_state == LIMIT_MaxHit))
            {
              stepperSetHome(i,
                  j->state.limit_step,
                  j->state.limit_state == LIMIT_MinHit ? j->min_limit : j->max_limit);
              state[i].stage = HOMING_PRE_FINAL;
            } else {
              printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
            }
          }
          break;
        case HOMING_PRE_FINAL:
          count--;
          state[i].stage = HOMING_FINAL;
          j->state.homed = TRUE;
          break;
        case HOMING_FINAL:
          break;
        }
      }
    }
  }
}
