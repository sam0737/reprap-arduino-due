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
 * @file    storage_config.h
 * @brief   Storgae Config related
 *
 * @addtogroup STORAGE
 * @{
 */

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

#define STRLEN(s) (sizeof(s)/sizeof(s[0]) - 1)

#define FILE_HEADER   "# RAD Config\r\n"

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void storageDumpConfigCore(FIL *fp) {
  UINT bw;

  f_write(fp, FILE_HEADER, STRLEN(FILE_HEADER), &bw);
  /* TODO */
}
/** @} */
