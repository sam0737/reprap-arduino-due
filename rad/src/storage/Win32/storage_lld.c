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


bool_t storageOpenFile(const char* filename)
{
  if (fileHandle != NULL)
    fclose(fileHandle);
  fileHandle = fopen(filename, "r");
  if (fileHandle == NULL)
    return FALSE;
  return fileHandle != NULL;
}

int storageReadLine(char* buf, int len)
{
  if (fileHandle == NULL)
    return STORAGE_ERROR;
  if (feof(fileHandle))
    return STORAGE_EOF;
  if (fgets(buf, len, fileHandle) == NULL)
    return STORAGE_ERROR;

  int newlen = strlen(buf);
  if (newlen == len - 1 && buf[newlen] != '\n')
  {
    int c;
    do
    {
      c = fgetc(fileHandle);
    } while (c >= 0 && c != '\n');
  }
  return newlen;
}

bool_t storageDumpConfig(void){ return TRUE; }

/** @} */
