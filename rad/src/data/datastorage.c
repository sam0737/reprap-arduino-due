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

#include "ch.h"
#include "chprintf.h"
#include "rad.h"

#define STORAGE_START 1
#define STORAGE_STOP 2
EventSource storage_evt;

#define COMMAND_LENGTH 128

typedef struct {
  EventSource ack_evt;
  EventListener ack_listener;
  EventListener status_listener;
  msg_t ack_mbox_buffer[COMMAND_BUFFER_SIZE + 1];
  Mailbox ack_mbox;

  decode_context_t decode_context;
  parse_context_t parse_context;

  bool_t enabled;

  char buf[COMMAND_LENGTH];
  PrinterCommand command;
} StorageContext;

static WORKING_AREA(waDataStorage, 128 + sizeof(StorageContext));

static void process_new_line(StorageContext* c) {

  PrinterCommand* cmd = &c->command;

  bool_t valid = gcodeDecode(cmd, c->buf, &c->decode_context);

  RAD_DEBUG_PRINTF("STORAGE: %s\n", c->buf);
  if (!valid) {
    printerEstop(L_PRINTER_STORAGE_GCODE_ERROR);
    return;
  }

  switch (cmd->code) {
  case 10999: // Estop clear
    printerEstopClear();
    break;
  case 10112: // EStop
    printerEstop(L_PRINTER_STOPPED_BY_STORAGE);
    break;
  }

  if (cmd->type & COMMANDTYPE_Action) {
    cmd->ack_mbox = &c->ack_mbox;
    cmd->ack_evt = &c->ack_evt;
    printerPushCommand(PRINTINGSOURCE_Storage, cmd);
  }
}

static msg_t threadDataStorage(void* arg) {
  chRegSetThreadName("data-storage");

  StorageContext c;
  memset(&c, 0, sizeof(StorageContext));
  chMBInit(&c.ack_mbox, c.ack_mbox_buffer, COMMAND_BUFFER_SIZE + 1);
  chEvtInit(&c.ack_evt);
  chEvtRegisterMask(&c.ack_evt, &c.ack_listener, 1);
  chEvtRegisterMaskWithFlags(&storage_evt, &c.status_listener, 2,
      STORAGE_START | STORAGE_STOP);

  while (1) {
    eventmask_t events = chEvtWaitAnyTimeout(ALL_EVENTS, c.enabled ? TIME_IMMEDIATE : MS2ST(500));
    if (events & 1) {
      PrinterCommand* ack_command;
      while (chMBFetch(&c.ack_mbox, (msg_t*) &ack_command, TIME_IMMEDIATE)
          == RDY_OK) {
        printerFreeCommand(ack_command);
      }
    }
    if (events & 2) {
      flagsmask_t flags = chEvtGetAndClearFlags(&c.status_listener);
      if (flags & STORAGE_START) {
        if (printerTryAcquire(PRINTINGSOURCE_Storage))
        {
          c.enabled = TRUE;
          printerTimeStart();
        }
      }
      if (flags & STORAGE_STOP) {
        c.enabled = FALSE;
        printerRelease(PRINTINGSOURCE_Storage);
      }
    }
    if (c.enabled)
    {
      if (printerIsEstopped()) {
        c.enabled = FALSE;
        continue;
      }

      int len = storageReadLine(c.buf, COMMAND_LENGTH);
      if (len == STORAGE_ERROR) {
        c.enabled = FALSE;
        printerEstop(L_STORAGE_FILE_READ_ERROR);
        continue;
      }
      if (len == STORAGE_EOF) {
        c.enabled = FALSE;
        printerRelease(PRINTINGSOURCE_Storage);
        continue;
      }
      gcodeResetParseContext(&c.parse_context);
      if (strlen(c.buf) == COMMAND_LENGTH - 1)
      {
        printerEstop(L_PRINTER_LINE_TOO_LONG);
        continue;
      }
      char* ptr = c.buf;
      while (*ptr != 0) {
        char i = gcodeFilterCharacter(*ptr, &c.parse_context);
        *ptr = i == 0 ? ' ' : i;
        ptr++;
      }
      process_new_line(&c);
    }
  }
  return 0;
}

void dataStorageSelect(const char* filename)
{
  if (printerGetState() != PRINTERSTATE_Standby)
  {
    printerEstop(L_STORAGE_NOT_IN_STANDBY);
    return;
  }
  if (!storageOpenFile(filename)) {
    printerEstop(L_STORAGE_FILE_OPEN_ERROR);
    return;
  }
}

void dataStoragePrintStart(void)
{
  chEvtBroadcastFlags(&storage_evt, STORAGE_START);
}

void dataStoragePrintStop(void)
{
  chEvtBroadcastFlags(&storage_evt, STORAGE_STOP);
}

void dataStorageInit(void)
{
  chEvtInit(&storage_evt);
  chThdCreateStatic(waDataStorage, sizeof(waDataStorage), NORMALPRIO - 20,
      threadDataStorage, NULL);
}
