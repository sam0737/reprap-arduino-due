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
  HOMING_SEARCH_AT_LIMIT = 2,
  HOMING_SEARCH_BACKOFF = 3,
  HOMING_LATCH_RELEASE = 4,
  HOMING_LATCH_HIT = 5,
  HOMING_PRE_FINAL = 6,
  HOMING_FINAL = 7,
} HomingStage;

typedef struct {
  HomingStage stage;
  float last_pos;
} HomingState;

static void movement_set_single_joint(PlannerJointMovement* m, uint8_t joint_id, float velocity)
{
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++)
  {
    m->joints[i] = i == joint_id ? velocity : NAN;
  }
}

static void actionHoming(uint32_t joint_mask)
{
  RAD_DEBUG_PRINTF("HOMING: Mask: %d\n", joint_mask);
  HomingState state[RAD_NUMBER_JOINTS];
  int8_t sequence = -1;
  uint8_t count;
  while (sequence < RAD_NUMBER_JOINTS) {
    count = 0;
    PlannerJointMovement m;
    memset(&m, 0, sizeof(PlannerJointMovement));
    memset(&state, 0, sizeof(state));

    RadJointsState joints_state = stepperGetJointsState();
    for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++)
    {
      RadJoint* j = &machine.kinematics.joints[i];
      if (j->home_sequence != sequence ||
          j->home_search_vel == 0 ||
          j->home_latch_vel == 0) continue;

      RadJointState* js = &joints_state.joints[i];

      if (sequence == -1 || (joint_mask & (1 << i))) {
        count++;
        stepperClearStopped(i);
        stepperResetOldLimitState(i);
        if ((j->home_search_vel < 0 && (js->limit_state & LIMIT_MinHit)) ||
            (j->home_search_vel > 0 && (js->limit_state & LIMIT_MaxHit)))
        {
          state[i].stage = HOMING_SEARCH_AT_LIMIT;
        } else {
          m.joints[i] = j->home_search_vel;
          state[i].last_pos = js->pos;
          state[i].stage = HOMING_SEARCH;
        }
      }
    }

    RAD_DEBUG_PRINTF("HOMING: Sequence: %d, No of joints to home: %d\n", sequence, count);

    sequence++;
    /* Go to the next sequence */
    if (count == 0) continue;

    m.rapid = 1;
    m.stop_on_limit_changes = TRUE;
    plannerSetJointVelocity(&m);

    while (count > 0)
    {
      if (printerIsEstopped()) return;
      chThdSleepMilliseconds(50);
      RadJointsState joints_state = stepperGetJointsState();
      for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++)
      {
        RadJoint* j = &machine.kinematics.joints[i];
        RadJointState* js = &joints_state.joints[i];
        switch (state[i].stage)
        {
        case HOMING_NONE:
          continue;
        case HOMING_SEARCH:
          if (fabs(state[i].last_pos - js->pos) >
              (j->max_limit - j->min_limit) * 1.2) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
            return;
          }
          if (!js->stopped) continue;
          stepperClearStopped(i);
          if (js->limit_state == LIMIT_Normal && js->changed_limit_state == LIMIT_Normal) {
            movement_set_single_joint(&m, i, j->home_search_vel);
            plannerSetJointVelocity(&m);
          } else {
            if ((j->home_search_vel < 0 && ((js->changed_limit_state | js->limit_state) & LIMIT_MinHit)) ||
                (j->home_search_vel > 0 && ((js->changed_limit_state | js->limit_state) & LIMIT_MaxHit)))
            {
              state[i].stage = HOMING_SEARCH_AT_LIMIT;
              continue;
            } else {
              printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
            }
          }
          break;
        case HOMING_SEARCH_AT_LIMIT:
          stepperClearStopped(i);
          stepperResetOldLimitState(i);
          state[i].last_pos = js->pos;
          if ((j->home_search_vel > 0 && j->home_latch_vel > 0) ||
              (j->home_search_vel < 0 && j->home_latch_vel < 0))
          {
            state[i].stage = HOMING_SEARCH_BACKOFF;
            movement_set_single_joint(&m, i, -j->home_search_vel);
          } else {
            state[i].stage = HOMING_LATCH_RELEASE;
            movement_set_single_joint(&m, i, j->home_latch_vel);
          }
          plannerSetJointVelocity(&m);
          break;
        case HOMING_SEARCH_BACKOFF:
          if (fabs(state[i].last_pos - js->pos) >
              fmax((j->max_limit - j->min_limit) / 20.0, fabs(j->home_search_vel) + 20)) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
          }
          if (!js->stopped) continue;
          stepperClearStopped(i);
          if (js->limit_state == LIMIT_Normal) {
            state[i].stage = HOMING_LATCH_HIT;
            movement_set_single_joint(&m, i, j->home_latch_vel);
            stepperResetOldLimitState(i);
            plannerSetJointVelocity(&m);
          } else {
            printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
          }
          break;
        case HOMING_LATCH_RELEASE:
          if (fabs(state[i].last_pos - js->pos) >
              fmax((j->max_limit - j->min_limit) / 20.0, fabs(j->home_search_vel) + 20)) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
          }
          if (!js->stopped) continue;
          stepperClearStopped(i);
          if (js->limit_state == LIMIT_Normal) {
            movement_set_single_joint(&m, i, 0);
            stepperSetHome(i,
                js->limit_step,
                j->home_search_vel < 0 ? j->min_limit : j->max_limit);
            state[i].stage = HOMING_PRE_FINAL;
          } else {
            printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
          }
          break;
        case HOMING_LATCH_HIT:
          if (fabs(state[i].last_pos - js->pos) >
              fmax((j->max_limit - j->min_limit) / 20.0, fabs(j->home_search_vel) + 20)) {
            printerEstop(L_HOMING_TRAVEL_LIMIT);
          }
          if (!js->stopped) continue;
          stepperClearStopped(i);
          if (js->limit_state != LIMIT_Normal) {
            if ((j->home_latch_vel < 0 && js->limit_state == LIMIT_MinHit) ||
                (j->home_latch_vel > 0 && js->limit_state == LIMIT_MaxHit))
            {
              movement_set_single_joint(&m, i, 0);
              stepperSetHome(i,
                  js->limit_step,
                  j->home_search_vel < 0 ? j->min_limit : j->max_limit);
              state[i].stage = HOMING_PRE_FINAL;
              continue;
            } else {
              printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
            }
          } else {
            printerEstop(L_HOMING_INCORRECT_LIMIT_HIT);
          }
          break;
        case HOMING_PRE_FINAL:
          count--;
          RAD_DEBUG_PRINTF("HOMING: Remaining no of joints to home: %d\n", count);
          state[i].stage = HOMING_FINAL;
          stepperSetHomed(i);
          break;
        case HOMING_FINAL:
          break;
        } /* stage case switch */
      } /* per axis processing */
    } /* while current stage not finished */
  } /* sequence */
  RAD_DEBUG_PRINTF("HOMING: Done\n");
} /* commandHoming */


static void commandHoming(void)
{
  uint32_t mask = 0;
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    for (uint8_t j = 0; j < RAD_NUMBER_AXES; j++) {
      if (!isnan(curr_command->axes_value[j]) &&
          machine.kinematics.axes[j].name ==
          machine.kinematics.joints[i].home_axis_name) {
        mask |= 1 << i;
        break;
      }
    }
  }
  if (mask == 0)
    mask = 0xFFFF;
  actionHoming(mask);
  printerSyncCommanded();
}
