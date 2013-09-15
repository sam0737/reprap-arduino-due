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
 * @file    storage.h
 * @brief   Storage header
 *
 * @addtogroup STORAGE
 * @{
 */
#ifndef _RAD_STORAGE_H
#define _RAD_STORAGE_H

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef enum {
  STORAGE_None = 0,
  STORAGE_Usb = 1,
  STORAGE_Local = 2
} RadStorageHost;

#ifdef __cplusplus
extern "C" {
#endif
  void storageInit(void);
  void storageUsbMount(void);
  void storageUsbUnmount(void);
  RadStorageHost storageGetHostState(void);
  bool_t storageDumpConfig(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_HMI_H */

/** @} */