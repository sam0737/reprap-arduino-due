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
static Mailbox command_main_mbox;
static msg_t command_alt_mbox_buffer[COMMAND_BUFFER_SIZE];
static Mailbox command_alt_mbox;

#define TOTAL_COMMAND_BUFFER_SIZE (COMMAND_BUFFER_SIZE * 2 + 3)
static PrinterCommand command_pool_buffer[TOTAL_COMMAND_BUFFER_SIZE];
static Semaphore command_pool_sem;
static MemoryPool command_pool;

static PrintingSource main_source;
static PrintingSource alt_source;

static PrinterCommand* curr_command;

static PrinterMode mode;
static PrinterMode mode_backup;
static PrinterState state;

static systime_t time_start;
static int32_t time_spent = -1;

static Mutex estop_mtx;
static uint8_t message_version = 1;
static char printer_message[64];

static WORKING_AREA(waPrinter, 512);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void printerSyncCommanded(void);

#include "command/move.h"
#include "command/homing.h"

#include "command/dispatch.h"

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
  chSemSignal(&command_pool_sem);
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
    if (state != PRINTERSTATE_Interrupted)
    {
      chMBFetch(&command_main_mbox, (msg_t*)&curr_command, MS2ST(100));
    } else {
      chMBFetch(&command_alt_mbox, (msg_t*)&curr_command, MS2ST(100));
    }

    if (curr_command == NULL) {
      continue;
    }

    if (!printerIsEstopped())
    {
      if (curr_command->type & COMMANDTYPE_TimeStart)
      {
        printerTimeStart();
      }
      printer_dispatch();

      if (curr_command->type & COMMANDTYPE_PrinterInterrupt)
      {
        printerSetState(PRINTERSTATE_Interrupted);
        mode_backup = mode;
      } else if (curr_command->type & COMMANDTYPE_PrinterResume)
      {
        // TODO: Restore axis position
        printerSetState(PRINTERSTATE_Printing);
        mode = mode_backup;
      }
    }

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

  chMBInit(&command_main_mbox, command_main_mbox_buffer, COMMAND_BUFFER_SIZE);
  chMBInit(&command_alt_mbox, command_alt_mbox_buffer, COMMAND_BUFFER_SIZE);

  chSemInit(&command_pool_sem, TOTAL_COMMAND_BUFFER_SIZE);
  chPoolInit(&command_pool, sizeof(PrinterCommand), NULL);
  chPoolLoadArray(&command_pool, command_pool_buffer, TOTAL_COMMAND_BUFFER_SIZE);

  chThdCreateStatic(waPrinter, sizeof(waPrinter), NORMALPRIO, threadPrinter, NULL);

  dataHostInit();
  dataStorageInit();
}

uint8_t printerGetActiveExtruder(void)
{
  return mode.tool;
}

float printerGetFeedrateMultiplier(void)
{
  return feedrate_multiplier;
}
void printerSetFeedrateMultiplier(const float value)
{
  feedrate_multiplier =
      value < 0.1 ? 0.1 :
      value > 5 ? 5 :
      value;
}
float printerGetFlowMultiplier(void)
{
  return flow_multiplier;
}
void printerSetFlowMultiplier(const float value)
{
  flow_multiplier =
      value < 0.1 ? 0.1 :
      value > 5 ? 5 :
      value;
}

void printerRelease(const PrintingSource source)
{
  chSysLock();
  if (++message_version == 0)
    message_version = 1;
  if (source == main_source) {
    main_source = PRINTINGSOURCE_None;
    if (state == PRINTERSTATE_Printing)
      state = PRINTERSTATE_Standby;
    printerTimeStopI();
  } else if (source == alt_source) {
    alt_source = PRINTINGSOURCE_None;
  }
  chSysUnlock();
}

bool_t printerTryAcquire(const PrintingSource source)
{
  bool_t result = FALSE;
  chSysLock();
  if (++message_version == 0)
    message_version = 1;
  if (state == PRINTERSTATE_Standby) {
    state = PRINTERSTATE_Printing;
    main_source = source;
  }

  if (main_source == source) {
    result = TRUE;
  } else {
    if (state == PRINTERSTATE_Interrupting ||
        state == PRINTERSTATE_Interrupted) {
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

PrintingSource printerGetMainSource(void)
{
  PrintingSource s;
  chSysLock();
  s = main_source;
  chSysUnlock();
  return s;
}

PrinterCommand* printerAllocateCommand(void)
{
  chSemWait(&command_pool_sem);
  PrinterCommand* command = chPoolAlloc(&command_pool);
  return command;
}

void printerInterrupt(const PrintingSource source)
{
  chSysLock();
  if ((state == PRINTERSTATE_Printing || state == PRINTERSTATE_Standby) &&
      source != main_source)
  {
    state = PRINTERSTATE_Interrupting;
    alt_source = PRINTINGSOURCE_None;

    PrinterCommand cmd;
    gcodeInitializeCommand(&cmd);
    cmd.type = COMMANDTYPE_PrinterInterrupt;
    printerPushCommand(main_source, &cmd);
  }
  chSysUnlock();
}

void printerResume(const PrintingSource source)
{
  chSysLock();
  if (state == PRINTERSTATE_Interrupted &&
      (source == alt_source || alt_source == PRINTINGSOURCE_None))
  {
    PrinterCommand cmd;
    gcodeInitializeCommand(&cmd);
    cmd.type = COMMANDTYPE_PrinterResume;
    printerPushCommand(PRINTINGSOURCE_None, &cmd);
  }
  chSysUnlock();
}

void printerPushCommand(const PrintingSource source, const PrinterCommand* command) {
  PrinterCommand* new_command = printerAllocateCommand();
  memcpy(new_command, command, sizeof(PrinterCommand));

  if (state != PRINTERSTATE_Interrupted || source == main_source) {
    chMBPost(&command_main_mbox, (msg_t) new_command, TIME_INFINITE);
  } else {
    chMBPost(&command_alt_mbox, (msg_t) new_command, TIME_INFINITE);
  }
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
  if (++message_version == 0)
    message_version = 1;

  if (state == PRINTERSTATE_Estopped)
    return;

  if (state == PRINTERSTATE_Printing && main_source == PRINTINGSOURCE_None)
    state = PRINTERSTATE_Standby;
  else
    state = new_state;
}

void printerSetState(PrinterState new_state)
{
  chSysLock();
  printerSetStateI(new_state);
  chSysUnlock();
}

void printerTimeStart(void)
{
  chSysLock();
  printerTimeStartI();
  chSysUnlock();
}

void printerTimeStartI(void)
{
  if (time_spent != -2)
  {
    time_spent = -2;
    time_start = chTimeNow();
  }
}

void printerTimeStopI(void)
{
  time_spent = printerTimeSpent();
}

int32_t printerTimeSpent(void)
{
  int32_t ret;
  chSysLock();
  ret = printerTimeSpentI();
  chSysUnlock();
  return ret;
}

int32_t printerTimeSpentI(void)
{
  if (time_spent == -2)
  {
    return (chTimeNow() - time_start) / CH_FREQUENCY;
  }
  return time_spent;
}

bool_t printerIsEstopped(void)
{
  bool_t r;
  chMtxLock(&estop_mtx);
  r = state == PRINTERSTATE_Estopped;
  chMtxUnlock();
  return r;
}


static void printerEstopCore(void)
{
  printerSetState(PRINTERSTATE_Estopped);

  if (++message_version == 0)
    message_version = 1;

  main_source = alt_source = PRINTINGSOURCE_None;
  beeperPlay(&tuneWarning);
  plannerEstop();
  temperatureAllZero();
  outputAllZero();

  chSysLock();
  printerTimeStopI();
  chSysUnlock();

  chMBReset(&command_main_mbox);
  chMBReset(&command_alt_mbox);
}

void printerEstop(const char *message)
{
  chMtxLock(&estop_mtx);

  if (message == NULL) {
    printer_message[0] = '\0';
  } else {
    strncpy(printer_message, message, sizeof(printer_message));
    printer_message[sizeof(printer_message) - 1] = '\0';
  }
  printerEstopCore();
  chMtxUnlock();
}

void printerEstopFormatted(const char *fmt, ...)
{
  chMtxLock(&estop_mtx);
  va_list arg;

  va_start(arg, fmt);
  vsnprintf(printer_message, sizeof(printer_message), fmt, arg);
  va_end(arg);

  printerEstopCore();
  chMtxUnlock();
}

void printerEstopClear(void)
{
  chMtxLock(&estop_mtx);
  if (state == PRINTERSTATE_Estopped)
  {
    plannerEstopClear();
    state = PRINTERSTATE_Standby;
    printer_message[0] = '\0';

    if (++message_version == 0)
      message_version = 1;
  }
  chMtxUnlock();
}

void printerSetMessage(const char* message)
{
  chMtxLock(&estop_mtx);
  if (message == NULL) {
    printer_message[0] = '\0';
  } else {
    strncpy(printer_message, message, sizeof(printer_message));
    printer_message[sizeof(printer_message) - 1] = '\0';
  }
  chMtxUnlock();
}

uint8_t printerGetMessage(uint8_t version, char* buffer, size_t size)
{
  chMtxLock(&estop_mtx);
  if (version != 0 && message_version == version)
  {
    chMtxUnlock();
    return message_version;
  }

  const char* m = printer_message;
  if (m[0] == '\0')
  {
    switch (printerGetState())
    {
      case PRINTERSTATE_Standby:
        m = L_UI_STATUS_READY;
        break;
      case PRINTERSTATE_Printing:
        m = L_UI_STATUS_PRINTING;
        break;
      case PRINTERSTATE_Interrupting:
        m = L_UI_STATUS_INTERRUPTING;
        break;
      case PRINTERSTATE_Interrupted:
      default:
        m = L_UI_STATUS_INTERRUPTED;
        break;
    }
  }
  strncpy(buffer, m, size);
  buffer[size - 1] = '\0';

  chMtxUnlock();
  return message_version;
}

/** @} */
