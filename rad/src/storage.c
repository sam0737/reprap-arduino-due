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

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waStorage, 256);
Mutex mutex;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void storageCheck(void) {
  chMtxLock(&mutex);
  if (blkIsTransferring(radboard.hmi.storage_device)) {
    chMtxUnlock();
    return;
  }
  if (machine.storage.state.host == STORAGE_None) {
    if (blkIsInserted(radboard.hmi.storage_device)) {
      machine.storage.state.host = STORAGE_Local;
    }
  } else if (machine.storage.state.host == STORAGE_Usb) {
    if (radboard.hmi.usb_msd->bbdp == NULL) {
      machine.storage.state.host = STORAGE_None;
    }
  } else if (machine.storage.state.host == STORAGE_Local) {
    if (!blkIsInserted(radboard.hmi.storage_device)) {
      machine.storage.state.host = STORAGE_None;
    }
  }
  chMtxUnlock();
}

void storageUsbMount(void) {
  chMtxLock(&mutex);
  msdReady(radboard.hmi.usb_msd, radboard.hmi.storage_device);
  machine.storage.state.host = STORAGE_Usb;
  chMtxUnlock();
}

void storageUsbUnmount(void) {
  chMtxLock(&mutex);
  msdEject(radboard.hmi.usb_msd);
  chMtxUnlock();
}

RadStorageState storageGetState(void) {
  chMtxLock(&mutex);
  RadStorageState state = machine.storage.state;
  chMtxUnlock();
  return state;
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

/** @} */
