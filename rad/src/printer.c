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
 * @file    printer.c
 * @brief   Printer control logic
 *
 * @addtogroup PRINTER
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static msg_t command_main_mbox_buffer[COMMAND_BUFFER_SIZE];

static Semaphore command_main_pool_sem;
static Mailbox command_main_mbox;

static msg_t command_alt_mbox_buffer[COMMAND_BUFFER_SIZE];

static Semaphore command_alt_pool_sem;
static Mailbox command_alt_mbox;

static PrinterCommand command_pool_buffer[COMMAND_BUFFER_SIZE * 2];
static MemoryPool command_pool;

static PrintingSource main_source;
static PrintingSource alt_source;

static PrinterCommand* curr_command;

static PrinterMode mode;
static PrinterState state;

static Mutex estop_mtx;
static const char* printer_estop_message = NULL;
static const char* printer_message = NULL;

static WORKING_AREA(waPrinter, 256);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void printerSyncCommanded(void);

#include "command/move.h"
#include "command/homing.h"

#include "command/dispatch.h"

static PrinterCommand* printer_new_command(void)
{
  chSemWait(&command_main_pool_sem);
  PrinterCommand* command = chPoolAlloc(&command_pool);
  return command;
}

static void printer_free_current_command(void)
{
  if (curr_command->ack_mbox)
  {
    chMBPost(curr_command->ack_mbox, (msg_t) curr_command, TIME_INFINITE);
    chEvtBroadcastFlags(curr_command->ack_evt, 1);
  } else
  {
    printerFreeCommand(curr_command);
  }
}

void printerFreeCommand(PrinterCommand* command)
{
  chPoolFree(&command_pool, command);
  chSemSignal(&command_main_pool_sem);
}

static msg_t threadPrinter(void *arg) {
  (void)arg;
  chRegSetThreadName("printer");

  beeperPlay(&tuneStartup);

  mode.rapid = RAPIDMODE_Feed;
  mode.distance = DISTANCEMODE_Absolute;
  mode.unit = UNITMODE_Millimeter;
  mode.extruder_distance = DISTANCEMODE_Absolute;
  mode.tool = 0;
  mode.feedrate = 30;

  while (1)
  {
    curr_command = NULL;
    chMBFetch(&command_main_mbox, (msg_t*)&curr_command, TIME_INFINITE);

    if (curr_command == NULL) {
      /* Should never reach this */
      chThdSleepMilliseconds(100);
      continue;
    }
    printer_dispatch();
    printer_free_current_command();
  }
  return 0;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void printerInit(void)
{
  chMtxInit(&estop_mtx);

  chSemInit(&command_main_pool_sem, COMMAND_BUFFER_SIZE);
  chMBInit(&command_main_mbox, command_main_mbox_buffer, COMMAND_BUFFER_SIZE);

  chSemInit(&command_alt_pool_sem, COMMAND_BUFFER_SIZE);
  chMBInit(&command_alt_mbox, command_alt_mbox_buffer, COMMAND_BUFFER_SIZE);

  chPoolInit(&command_pool, sizeof(PrinterCommand), NULL);
  chPoolLoadArray(&command_pool, command_pool_buffer, COMMAND_BUFFER_SIZE);

  chThdCreateStatic(waPrinter, sizeof(waPrinter), NORMALPRIO, threadPrinter, NULL);

  dataHostInit();
}

uint8_t printerGetActiveExtruder(void)
{
  return mode.tool;
}

void printerRelease(const PrintingSource source)
{
  chSysLock();
  if (state == PRINTERSTATE_Interrupted) {
    if (source == alt_source)
      alt_source = PRINTINGSOURCE_None;
  } else if (state == PRINTERSTATE_Printing) {
    if (source == main_source) {
      main_source = PRINTINGSOURCE_None;
      state = PRINTERSTATE_Standby;
    }
  }
  chSysUnlock();
}

bool_t printerTryAcquire(const PrintingSource source)
{
  bool_t result = FALSE;
  chSysLock();
  if (state == PRINTERSTATE_Standby) {
    state = PRINTERSTATE_Printing;
    main_source = source;
  }

  if (main_source == source) {
    result = TRUE;
  } else {
    if (state == PRINTERSTATE_Interrupted) {
      if (alt_source == PRINTINGSOURCE_None) {
        alt_source = source;
      }
      if (alt_source == source) {
        result = TRUE;
      }
    }
  }
  chSysUnlock();
  return result;
}

void printerPushCommand(const PrinterCommand* command) {
  PrinterCommand* new_command = printer_new_command();
  memcpy(new_command, command, sizeof(PrinterCommand));
  chMBPost(&command_main_mbox, (msg_t) new_command, TIME_INFINITE);
}

PrinterState printerGetStateI(void)
{
  return state;
}

PrinterState printerGetState(void)
{
  chSysLock();
  PrinterState now_state = state;
  chSysUnlock();
  return now_state;
}

void printerSetStateI(PrinterState new_state)
{
  if (state == PRINTERSTATE_Estopped)
    return;
  state = new_state;
}

void printerSetState(PrinterState new_state)
{
  chSysLock();
  printerSetStateI(new_state);
  chSysUnlock();
}

const char* printerIsEstopped(void)
{
  const char* message;
  chMtxLock(&estop_mtx);
  message = printer_estop_message;
  chMtxUnlock();
  return message;
}

void printerEstop(const char *message)
{
  chMtxLock(&estop_mtx);
  printerSetState(PRINTERSTATE_Estopped);
  printer_estop_message = message;
  main_source = alt_source = PRINTINGSOURCE_None;
  beeperPlay(&tuneWarning);
  plannerEstop();
  temperatureAllZero();
  outputAllZero();
  chMtxUnlock();
}

void printerEstopClear(void)
{
  chMtxLock(&estop_mtx);
  if (state == PRINTERSTATE_Estopped)
  {
    plannerEstopClear();
    state = PRINTERSTATE_Standby;
    printer_estop_message = NULL;
  }
  chMtxUnlock();
}

void printerSetMessage(const char* message)
{
  printer_message = message;
}

const char* printerGetMessage(void)
{
  const char* m = printer_estop_message;
  if (m == NULL)
    m = printer_message;
  return m;
}

/** @} */
