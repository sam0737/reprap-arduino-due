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

#include "rad.h"

#if RAD_TEST

void cmd_test_dump_block(BaseSequentialStream *chp, PlannerOutputBlock* block)
{
  chprintf(chp, "%s ",
      block->mode == BLOCK_Idle ? "Idle" :
      block->mode == BLOCK_Velocity ? "Vel" :
      block->mode == BLOCK_Positional ? "Pos" :
      block->mode == BLOCK_Estop ? "Estop" :
      block->mode == BLOCK_Reset ? "Reset" :
          "Estop_Clear"
      );

  if (block->mode == BLOCK_Positional) {
    uint8_t i;
    for (i = 0; i < RAD_NUMBER_AXES; i++) {
      chprintf(chp, "%9.3f ", block->p.target.joints[i]);
    }
    for (i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
      chprintf(chp, "%9.3f ", block->p.target.extruders[i]);
    }
    chprintf(chp, "V%6.2f X%6.2f MX%6.2f A%6.2f S%.3f Sa%.3f T%.3f",
        block->p.nominal_speed, block->p.exit_speed, block->p.max_exit_speed, block->p.acc,
        block->p.distance, block->p.decelerate_after,
        block->p.duration);
  }
  chprintf(chp, "\r\n");
}

void cmd_test_dump_all_blocks(BaseSequentialStream *chp)
{
  PlannerOutputBlock block;
  bool_t new_block = FALSE;
  while (1)
  {
    chSysLock();
    new_block = plannerMainQueueFetchBlockI(&block, BLOCK_Idle);
    if (!new_block) return;
    chSysUnlock();
    cmd_test_dump_block(chp, &block);
    block.mode = BLOCK_Idle;
  }
}

static void cmd_test_planner(BaseSequentialStream *chp, int argc, char *argv[]) {
  FILE *fp;
  char path[255];
  if (argc != 1) {
    chprintf(chp, "Usage: test_planner [filename]\r\n");
    return;
  }
  strcpy(path, "test/");
  strncat(path, argv[0], 200);
  fp = fopen(path, "r");

  if (fp == NULL) {
    chprintf(chp, "Failed to read the file %s\r\n", path);
    return;
  }

  int line = 0;
  char buffer[128];
  while (fgets(buffer, 128, fp))
  {
    line++;
    RAD_DEBUG_PRINTF("Line: %d: %s", line, buffer);
    //printerAddLine(buffer);
    //cmd_test_dump_all_blocks(chp);
  }
  fclose(fp);

  chThdSleep(100); // Sleep for printer thread to finish processing.
  RAD_DEBUG_PRINTF("Finished\n");
  //cmd_test_dump_all_blocks(chp);
}

#endif
