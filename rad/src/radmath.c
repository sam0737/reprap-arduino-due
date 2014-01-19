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
 * @file    radmath.c
 * @brief   RAD Math utilities
 *
 * @addtogroup RADMATH
 * @{
 */

#include "radmath.h"

float fast_inverse_square(float x)
{
  float xhalf = 0.5f*x;
  union
  {
    float x;
    int i;
  } u;
  u.x = x;
  u.i = 0x5f3759df - (u.i >> 1);
  /* The next line can be repeated any number of times to increase accuracy */
  u.x = u.x * (1.5f - xhalf * u.x * u.x);
  u.x = u.x * (1.5f - xhalf * u.x * u.x);
  return u.x;
}

float rad_strtof(const char *nptr, char **endptr) {
  float val = 0.0f;
  int d = 0;
  int sign = 1;

  if (!nptr) {
    return 0.0f;
  }

  if (*nptr == '+') {
    nptr++;
  } else if (*nptr == '-') {
    nptr++;
    sign = -1;
  }

  while (*nptr >= '0' && *nptr <= '9') {
    val = val * 10.0 + (*nptr - '0');
    nptr++;
  }

  if (*nptr == '.') {
    nptr++;
    while (*nptr >= '0' && *nptr <= '9') {
      val = val * 10.0 + (*nptr - '0');
      nptr++;
      d--;
    }
  }

  if (*nptr == 'E' || *nptr == 'e') {
    int e_sign = 1;
    int e_val = 0;

    nptr++;
    if (*nptr == '+') {
      nptr++;
    } else if (*nptr == '-') {
      nptr++;
      sign = -1;
    }

    while ((*nptr >= '0' && *nptr <= '9')) {
      e_val = e_val * 10 + (*nptr - '0');
      nptr++;
    }
    d += e_val * e_sign;
  }

  while (d > 0) {
    val *= 10.0;
    d--;
  }
  while (d < 0) {
    val *= 0.1;
    d++;
  }

  if (endptr) {
    *endptr = (char *) nptr;
  }

  return sign * val;
}

/** @} */
