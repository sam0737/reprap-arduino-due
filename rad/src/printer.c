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
#include "chprintf.h"

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

// static char axis_code[] = {' ', 'X', 'Y', 'Z'};

Semaphore command_pool_sem;
MemoryPool command_pool;
Mailbox command_mbox;

static PrinterCommand *curr_command;
char* printer_estop_message = NULL;

static uint8_t active_extruder;

static WORKING_AREA(waPrinterFetchSerial, 128);
static WORKING_AREA(waPrinter, 512);

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static void printerSyncCommanded(void);

#include "command/gcode_decode.h"
#include "command/move.h"
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

static void printer_normalize_command(void)
{
  bool_t in_comment = FALSE;
  for (uint8_t i = 0; i < COMMAND_LENGTH; i++)
  {
    char c = curr_command->payload[i];
    if (c == '\0') return;
    if (!in_comment)
    {
      if (c == '(') {
          curr_command->payload[i] = ' ';
          in_comment = TRUE;
      } else if (c == ';') {
        curr_command->payload[i] = '\0';
        return;
      }
    } else {
      curr_command->payload[i] = ' ';
      if (c == ')')
        in_comment = FALSE;
    }
  }
}

static void printer_dispatch(void)
{
  if (code_seen('G'))
  {
    switch ((int)code_value()){
    case 0:
    case 1:
      if (printerIsEstopped()) break;
      commandMove();
      break;
    /* Dwell */
    case 4:
      if (code_seen('P'))
        chThdSleepMilliseconds((int)code_value());
      break;
    /* Move to origin */
    case 28:
      printerEstopClear();
      commandHoming();
      break;
    /* Set Position */
    case 92:
      commandSetPosition();
    }
  } else if (code_seen('M'))
  {
    switch ((int)code_value()){
    case 80:
      powerPsuOn();
      break;
    case 81:
      powerPsuOff();
      break;
    case 999:
      printerEstopClear();
      break;
    }
  }
}

static msg_t threadPrinterFetchSerial(void *arg) {
  (void)arg;
  chRegSetThreadName("fetch-ser");

  static char* ptr;
  static char buf[COMMAND_LENGTH];
  static char in_comment = 0;

  ptr = buf;
  while (1)
  {
    msg_t c = chSequentialStreamGet(radboard.hmi.comm_channel);
    if (c < 0) {
      /* Should never reach this */
      chThdSleepMilliseconds(100);
      continue;
    }
    if (c == '\n' || c == '\r') {
      in_comment = 0;

      if (ptr == buf) continue;

      *ptr = '\0';
      // chprintf((BaseSequentialStream*)radboard.hmi.comm_channel, "ok %d\n", strlen(buf));
      printerAddLine(buf);
      ptr = buf;
      continue;

      /*
       * TODO Checksum, line number, Send ok
       */
    }

    if ((ptr - buf) >= COMMAND_LENGTH)
      continue;

    if (in_comment == 0)
    {
      if (c == '(') {
          in_comment = 1;
          continue;
      } else if (c == ';') {
        in_comment = 2;
        continue;
      }
    } else if (in_comment == 1 && c == ')') {
      in_comment = 0;
      continue;
    } else {
      continue;
    }

    if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    *(ptr++) = c;
  }
  return 0;
}

static msg_t threadPrinter(void *arg) {
  (void)arg;
  chRegSetThreadName("printer");

  beeperPlay(&tuneStartup);

  while (1)
  {
    curr_command = NULL;
    chMBFetch(&command_mbox, (msg_t*)&curr_command, TIME_INFINITE);

    if (curr_command == NULL) {
      /* Should never reach this */
      chThdSleepMilliseconds(100);
      continue;
    }
    printer_normalize_command();
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

uint8_t printerGetActiveExtruder(void)
{
  return active_extruder;
}

void printerAddLine(const char* line)
{
  PrinterCommand* cmd = printer_new_command();
  strcpy(cmd->payload, line);
  chMBPost(&command_mbox, (msg_t) cmd, TIME_INFINITE);
}

bool_t printerIsEstopped(void)
{
  return (printer_estop_message != NULL);
}

void printerEstop(char *message)
{
  printer_estop_message = message;
  beeperPlay(&tuneWarning);
  plannerEstop();
  temperatureAllZero();
  outputAllZero();
}

void printerEstopClear(void)
{
  printer_estop_message = NULL;
  plannerEstopClear();
}

/** @} */
