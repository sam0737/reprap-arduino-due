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
 * @file    storage.c
 * @brief   Storgae
 *
 * @addtogroup STORAGE
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "storage.h"

#if RAD_STORAGE

#include "ff.h"
/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

#include "storage_config.h"
static WORKING_AREA(waStorage, 512);
static Mutex mutex;

static RadStorageHost host;

#define SETTINGS_FILENAME "rad.cfg"
FATFS fsWorkArea;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void storageCheck(void) {
  chMtxLock(&mutex);
  if (blkIsTransferring(radboard.hmi.storage_device)) {
    chMtxUnlock();
    return;
  }
  if (host == STORAGE_None) {
    if (blkIsInserted(radboard.hmi.storage_device)) {
      host = STORAGE_Local;
      f_mount(0, &fsWorkArea);
    }
  } else if (host == STORAGE_Usb) {
#if HAL_USE_MSD
    if (radboard.hmi.usb_msd->bbdp == NULL) {
      host = STORAGE_None;
    }
#endif
  } else if (host == STORAGE_Local) {
    if (!blkIsInserted(radboard.hmi.storage_device)) {
      host = STORAGE_None;
      f_mount(0, NULL);
    }
  }
  chMtxUnlock();
}

static msg_t threadStorage(void *arg) {
  (void)arg;
  chRegSetThreadName("storage");

  while (TRUE) {
    storageCheck();
    chThdSleepMilliseconds(100);
  }
  return 0;
}

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void storageInit(void)
{
  chMtxInit(&mutex);
  if (radboard.hmi.storage_device == NULL)
    return;
  chThdCreateStatic(waStorage, sizeof(waStorage), NORMALPRIO - 2, threadStorage, NULL);
}

void storageUsbMount(void) {
#if HAL_USE_MSD
  chMtxLock(&mutex);
  msdReady(radboard.hmi.usb_msd, radboard.hmi.storage_device);
  host = STORAGE_Usb;
  chMtxUnlock();
#endif
}

void storageUsbUnmount(void) {
#if HAL_USE_MSD
  chMtxLock(&mutex);
  msdEject(radboard.hmi.usb_msd);
  chMtxUnlock();
#endif
}

RadStorageHost storageGetHostState(void) {
  chMtxLock(&mutex);
  RadStorageHost _host = host;
  chMtxUnlock();
  return _host;
}

bool_t storageDumpConfig(void) {
  FIL f;

  chMtxLock(&mutex);
  if (host != STORAGE_Local)
    goto failed;
  if (f_open(&f, SETTINGS_FILENAME, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    goto failed;
  storageDumpConfigCore(&f);
  if (f_close(&f) != FR_OK)
    goto failed;
  chMtxUnlock();
  return TRUE;
failed:
  chMtxUnlock();
  return FALSE;
}

#else
void storageInit(void) {}
void storageUsbMount(void) {}
void storageUsbUnmount(void) {}
RadStorageHost storageGetHostState(void){ return STORAGE_None; }
bool_t storageDumpConfig(void){ return TRUE; }
#endif

/** @} */
