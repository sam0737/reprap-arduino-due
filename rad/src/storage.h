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

#include "storage_lld.h"

typedef enum {
  FILETYPE_File = 0,
  FILETYPE_Directory = 1
} RadFileType;

typedef struct {
  char* filename;
  RadFileType type;
} RadFileInfo;

typedef enum {
  STORAGE_None = 0,
  STORAGE_Usb = 1,
  STORAGE_Local = 2
} RadStorageHost;

#define STORAGE_ERROR -2
#define STORAGE_EOF -1

#ifdef __cplusplus
extern "C" {
#endif
  void storageInit(void);
  void storageUsbMount(void);
  void storageUsbUnmount(void);
  RadStorageHost storageGetHostState(void);

  void storageChangeDir(const char* path);
  void storageOpenDir(void);
  bool_t storageFetchFileInfo(RadFileInfo* file);
  void storageCloseDir(void);

  bool_t storageOpenFile(const char* filename, uint32_t* sizep);
  int storageReadChar(void);

  bool_t storageDumpConfig(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_HMI_H */

/** @} */
