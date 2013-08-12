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

#include "printer.h"

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

#define COMMAND_LENGTH 128
#define COMMAND_BUFFER_SIZE 8

typedef struct {
  char payload[COMMAND_LENGTH];
} PrinterCommand;

static PrinterCommand command_pool_buffer[COMMAND_BUFFER_SIZE];
static msg_t command_mbox_buffer[COMMAND_BUFFER_SIZE];

static char axis_code[] = {' ', 'X', 'Y', 'Z'};

Semaphore command_pool_sem;
MemoryPool command_pool;
Mailbox command_mbox;

static PrinterCommand *curr_command;
char* printer_estop_message = NULL;

static WORKING_AREA(waPrinterFetchSerial, 128);
static WORKING_AREA(waPrinter, 256);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

#include "command/gcode_decode.h"
#include "command/homing.h"

static PrinterCommand* printer_new_command(void)
{
  chSemWait(&command_pool_sem);
  PrinterCommand* command = chPoolAlloc(&command_pool);
  return command;
}

static void printer_free_current_command(void)
{
  chPoolFree(&command_pool, curr_command);
  chSemSignalI(&command_pool_sem);
}

static void printer_dispatch(void)
{
  if (code_seen('G'))
  {
    switch ((int)code_value()){
    case 29:
      commandHoming(0xFFFF);
      break;
    }
  } else if (code_seen('M'))
  {
    switch ((int)code_value()){
    case 999:
      printerEstopClear();
      break;
    }
  }
}

static msg_t threadPrinterFetchSerial(void *arg) {
  (void)arg;
  chRegSetThreadName("printer-ser");

  static char* ptr;
  static char buf[COMMAND_LENGTH];
  static bool_t comment_mode;

  while (1)
  {
    msg_t c = chSequentialStreamGet(radboard.hmi.comm_channel);
    if (c < 0) {
      /* Should never reach this */
      chThdSleepMilliseconds(100);
      continue;
    }
    if (c == '\n' || c == '\r') {
      comment_mode = FALSE;

      buf[COMMAND_LENGTH - 1] = 0;
      PrinterCommand* cmd = printer_new_command();
      strcpy(cmd->payload, buf);
      chMBPost(&command_pool, (msg_t) cmd, TIME_INFINITE);
      ptr = buf;

      /*
       * Checksum, Send ok
       */
    }

    if (comment_mode) continue;
    if (c == ';' || (ptr - buf) >= COMMAND_LENGTH) {
      comment_mode = TRUE;
      continue;
    }

    if (c >= 'a' && c <= 'z') c-= 'a' - 'A';
    *(ptr++) = c;
  }
  return 0;
}

static msg_t threadPrinter(void *arg) {
  (void)arg;
  chRegSetThreadName("printer");

  while (1)
  {
    curr_command = NULL;
    chMBFetch(&command_mbox, (msg_t*)&curr_command, TIME_INFINITE);

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
  chPoolInit(&command_pool, sizeof(PrinterCommand), NULL);
  chPoolLoadArray(&command_pool, command_pool_buffer, COMMAND_BUFFER_SIZE);
  chMBInit(&command_mbox, command_mbox_buffer, COMMAND_BUFFER_SIZE);
  chSemInit(&command_pool_sem, COMMAND_BUFFER_SIZE);

  chThdCreateStatic(waPrinter, sizeof(waPrinter), NORMALPRIO, threadPrinter, NULL);
  chThdCreateStatic(waPrinterFetchSerial, sizeof(waPrinterFetchSerial), NORMALPRIO,
      threadPrinterFetchSerial, NULL);
}

void printerAddLine(char* line)
{
  PrinterCommand* cmd = printer_new_command();
  strcpy(cmd->payload, line);
  chMBPost(&command_mbox, (msg_t) cmd, TIME_INFINITE);
}

void printerEstop(char *message)
{
  printer_estop_message = message;
  plannerEstop();
}

void printerEstopClear(void)
{
  printer_estop_message = NULL;
  plannerEstopClear();
}

/** @} */
