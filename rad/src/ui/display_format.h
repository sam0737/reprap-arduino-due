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
 * @file    ui/display_format.h
 * @brief   User interface Display format header
 *
 * @addtogroup UI
 * @{
 */
#ifndef _RAD_UI_DISPLAY_FORMAT_H
#define _RAD_UI_DISPLAY_FORMAT_H

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
char *itostr3(const int x);
char *itostr3left(const int xx);
char *itostr2(const int xx);
char *ftostr42(const float x);
char *ftostr42best(const float x);
char *itospace(uint8_t space, int xx);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_UI_DISPLAY_FORMAT_H */

/** @} */
