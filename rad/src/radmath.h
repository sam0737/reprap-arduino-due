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
 * @file    src/radmath.h
 * @brief   RAD Math header.
 *
 * @addtogroup RADMATH
 * @{
 */

#ifndef _RADMATH_H_
#define _RADMATH_H_

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  float fast_inverse_square(float);
  float rad_strtof(const char *nptr, char **endptr);
#ifdef __cplusplus
}
#endif

#endif  /* _RADMATH_H_ */

/** @} */
