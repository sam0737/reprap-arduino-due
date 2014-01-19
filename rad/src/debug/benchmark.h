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
#include <stdlib.h>

static void cmd_benchmark(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  TimeMeasurement tm;
  tmObjectInit(&tm);
  tmStartMeasurement(&tm);
  float x;
  for (float i = 1; i <= 100000; i++) {
    x = (float)1/sqrt(i);
  }
  tmStopMeasurement(&tm);
  chprintf(chp, "%f, T1 = %ld\r\n", x, RTT2MS(tm.last));
  tmStartMeasurement(&tm);
  for (float i = 1; i <= 100000; i++) {
    x = fast_inverse_square(i);
  }
  tmStopMeasurement(&tm);
  chprintf(chp, "%f, T2 = %ld\r\n", x, RTT2MS(tm.last));
}
