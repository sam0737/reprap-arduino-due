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
 * @file    storage_lld.c
 * @brief   Storgae
 *
 * @addtogroup STORAGE
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

void storageInit(void) {}
void storageUsbMount(void) {}
void storageUsbUnmount(void) {}
RadStorageHost storageGetHostState(void){ return STORAGE_None; }

void storageChangeDir(const char* path) { (void) path; }
void storageOpenDir(void) { }
bool_t storageFetchFileInfo(RadFileInfo* file) { (void) file; return FALSE; }
void storageCloseDir(void) { }

bool_t storageOpenFile(const char* filename, uint32_t* sizep)
{
  (void) filename;
  (void) sizep;
  return FALSE;
}

int storageReadChar(void) { return STORAGE_ERROR; }

bool_t storageDumpConfig(void){ return TRUE; }

/** @} */
