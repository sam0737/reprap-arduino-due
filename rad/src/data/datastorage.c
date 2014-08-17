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

  uint32_t line;
  uint32_t processed_len;

  char* ptr;
  char buf[COMMAND_LENGTH];
  PrinterCommand command;
} StorageContext;

uint32_t file_size;
uint32_t processed_len;

static WORKING_AREA(waDataStorage, 256 + sizeof(StorageContext));

static void process_new_line(StorageContext* c) {

  PrinterCommand* cmd = &c->command;

  bool_t valid = gcodeDecode(cmd, c->buf, &c->decode_context);

  RAD_DEBUG_PRINTF("STORAGE: %s\n", c->buf);
  if (!valid) {
    printerEstopFormatted(L_PRINTER_STORAGE_GCODE_ERROR, c->line);
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
  (void) arg;
  chRegSetThreadName("data-storage");

  StorageContext c;
  memset(&c, 0, sizeof(StorageContext));
  chMBInit(&c.ack_mbox, c.ack_mbox_buffer, COMMAND_BUFFER_SIZE + 1);
  chEvtInit(&c.ack_evt);
  chEvtRegisterMask(&c.ack_evt, &c.ack_listener, 1);
  chEvtRegisterMaskWithFlags(&storage_evt, &c.status_listener, 2,
      STORAGE_START | STORAGE_STOP);

  while (1) {
    eventmask_t events = chEvtWaitAnyTimeout(ALL_EVENTS, c.line > 0 ? MS2ST(5) : MS2ST(500));
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
          c.processed_len = 0;
          c.line = 1;
          c.ptr = c.buf;
          gcodeResetParseContext(&c.parse_context);
          printerTimeStart();
        }
      }
      if (flags & STORAGE_STOP) {
        c.line = 0;
        printerRelease(PRINTINGSOURCE_Storage);
      }
    }
    if (c.line)
    {
      if (printerIsEstopped()) {
        c.line = 0;
        continue;
      }

      while (1)
      {
        int i = storageReadChar();

        if (i == STORAGE_ERROR) {
        printerEstopFormatted(L_STORAGE_FILE_READ_ERROR, c.line);
          break;
      }
        if (i == STORAGE_EOF) {
        printerRelease(PRINTINGSOURCE_Storage);
          break;
      }
        c.processed_len++;

        // End of line
        if (i == '\n' || i == '\r')
        {
          c.line++;
      gcodeResetParseContext(&c.parse_context);
          if (c.ptr == c.buf) continue;

          *c.ptr = '\0';
          process_new_line(&c);
          c.ptr = c.buf;
          break;
        }

        if ((c.ptr - c.buf) >= COMMAND_LENGTH)
      {
          printerEstopFormatted(L_PRINTER_FILE_LINE_TOO_LONG, c.line);
          break;
      }

        i = gcodeFilterCharacter(i, &c.parse_context);
        if (i)
          *(c.ptr++) = i;
      }
      chSysLock();
      processed_len = c.processed_len;
      chSysUnlock();
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
  chSysLock();
  if (!storageOpenFile(filename, &file_size)) {
    printerEstop(L_STORAGE_FILE_OPEN_ERROR);
    chSysUnlock();
    return;
  }
  chSysUnlock();
  uiStateSetActiveFilename(filename);
}

uint32_t dataStorageGetCurrentFileSize(void)
{
  uint32_t s;
  chSysLock();
  s = file_size;
  chSysUnlock();
  return s;
}

uint32_t dataStorageGetCurrentProcessed(void)
{
  uint32_t s;
  chSysLock();
  s = processed_len;
  chSysUnlock();
  return s;
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
