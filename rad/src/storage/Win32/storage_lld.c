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

RadStorageHost storageGetHostState(void)
{
  return STORAGE_Local;
}

void storageChangeDir(const char* path)
{
  SetCurrentDirectory(path);
}

static WIN32_FIND_DATA findFileData;
static HANDLE dirHandle;
static FILE* fileHandle;
static bool_t firstCall;

void storageOpenDir()
{
  dirHandle = FindFirstFile("*", &findFileData);
  firstCall = TRUE;
}

bool_t storageFetchFileInfo(RadFileInfo* file)
{
  if (dirHandle == INVALID_HANDLE_VALUE)
    return FALSE;

  if (!firstCall) {
    if (!FindNextFile(dirHandle, &findFileData))
      return FALSE;
  }

  firstCall = FALSE;
  file->filename = findFileData.cFileName;
  file->type =
      findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
          FILETYPE_Directory :
          FILETYPE_File;

  return TRUE;
}

void storageCloseDir()
{
  if (dirHandle == INVALID_HANDLE_VALUE) return;
  FindClose(dirHandle);
  dirHandle = INVALID_HANDLE_VALUE;
}

bool_t storageOpenFile(const char* filename, uint32_t* sizep)
{
  if (fileHandle != NULL)
    fclose(fileHandle);
  fileHandle = fopen(filename, "r");
  if (fileHandle == NULL)
    return FALSE;

  if (fileHandle != NULL)
  {
    struct stat stat_buf;
    if (fstat(fileno(fileHandle), &stat_buf) == 0)
      *sizep = stat_buf.st_size;
  }

  return fileHandle != NULL;
}

int storageReadChar(void)
{
  if (fileHandle == NULL)
    return STORAGE_ERROR;
  if (feof(fileHandle))
    return STORAGE_EOF;

  int c = fgetc(fileHandle);
  if (c == EOF) return STORAGE_EOF;

  return c;
}

bool_t storageDumpConfig(void){ return TRUE; }

/** @} */
