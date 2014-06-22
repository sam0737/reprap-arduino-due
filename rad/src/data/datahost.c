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

#define COMMAND_LENGTH 128
#define hostprintf(c, ...) \
  RAD_DEBUG_PRINTF(__VA_ARGS__); \
  chprintf((BaseSequentialStream*)((c)->channel), __VA_ARGS__);

#define REPORT_INTERVAL (S2ST(0.2))
#define IDLE_INTERVAL (MS2ST(500))

typedef struct {
  BaseAsynchronousChannel* channel;
  EventSource ack_evt;
  EventListener ack_listener;
  EventListener host_listener;
  msg_t ack_mbox_buffer[COMMAND_BUFFER_SIZE + 1];
  Mailbox ack_mbox;

  char* ptr;
  char buf[COMMAND_LENGTH];
  decode_context_t decode_context;
  parse_context_t parse_context;
  PrinterCommand command;
  systime_t last_report_time;
  systime_t last_busy_time;
  uint8_t busy;

  int32_t last_received_line;
} HostContext;

static WORKING_AREA(waDataHost, 128 + sizeof(HostContext));

static void process_new_line(HostContext* c) {
  PrinterCommand* cmd = &c->command;

  bool_t valid = gcodeDecode(cmd, c->buf, &c->decode_context);
  if (cmd->line >= 0)
  {
    // Line number change request
    if (cmd->line == c->last_received_line + 1 ||
        // Line number change request
        cmd->code == 10110)
    {
      c->last_received_line = cmd->line;
    } else if (cmd->line > c->last_received_line + 1)
    {
      // Out of order lines received
      hostprintf(c, "rs %d Resend:%d\n", c->last_received_line + 1, c->last_received_line + 1);
      return;
    } else {
      // Check current status?
      hostprintf(c, c->busy ? "busy\n" : "ok\n");
      return;
    }
  }

  RAD_DEBUG_PRINTF("HOST: %s\n", c->buf);
  if (!valid)
  {
    const char* message;
    if (!(message = printerIsEstopped()))
      printerEstop((message = L_PRINTER_HOST_GCODE_ERROR));

    hostprintf(c, "ok\n!! %s.", message, cmd->line);
    if (cmd->line > 0)
      hostprintf(c, " Line %d\n", cmd->line);
    hostprintf(c, "\n");
    return;
  }

  switch (cmd->code)
  {
    case 10999: // Estop clear
    case 28: // Homing
      printerEstopClear();
      break;
  }

  /* Host specific Gcode */
  switch (cmd->code)
  {
    case 10112: // EStop
      printerEstop(L_PRINTER_STOPPED_BY_HOST);
      hostprintf(c, "ok\n");
      break;
    case 10105: // Get temperature
    case 10114: // Get position
      // Once these commands are received, we will send temp/position report ever now and then
      c->last_report_time = chTimeNow() - REPORT_INTERVAL;
      hostprintf(c, "ok\n");
      break;
    case 10115: // Host capability report
      hostprintf(c,
          "ok FIRMWARE_NAME:RAD(marlin) FIRMWARE_URL:http%%3A//rad.hellosam.net/ EXTRUDER_COUNT:%d\n",
          RAD_NUMBER_EXTRUDERS);
      break;
    default:
      if (cmd->type & COMMANDTYPE_UnknownCode)
      {
        if (cmd->code < 10000) {
          hostprintf(c, "!! Unknown G code - %d\n", cmd->code);
        } else {
          hostprintf(c, "!! Unknown M code - %d\n", cmd->code - 10000);
        }
      }
      if (!(cmd->type & COMMANDTYPE_Action))
      {
        hostprintf(c, "ok #%d [%d] {-}\n", cmd->line, cmd->code);
      }
  }

  if (cmd->type & COMMANDTYPE_Action)
  {
    if ((cmd->type & COMMANDTYPE_SyncAction) == COMMANDTYPE_SyncAction) 
    {
      if (c->busy)
      {
        hostprintf(c, "busy. Please enable ping-pong.\n", cmd->line);
        return;
      }
      if (!printerTryAcquire(PRINTINGSOURCE_Host)) 
      {
        switch (printerGetState()) 
        {
          case PRINTERSTATE_Printing:
            hostprintf(c, "!! Please pause first\n");
            break;
          case PRINTERSTATE_Interrupting:
          case PRINTERSTATE_Interrupted:
            hostprintf(c, "!! Printer is under manual control\n");
            break;
          case PRINTERSTATE_Estopped:
            hostprintf(c, "!! %s\n", printerGetMessage());
            break;
          default:
            // Impossible to be here
            break;
        }
        chThdSleepMilliseconds(1000); // Delay to avoid host bursting us
        hostprintf(c, "ok\n");
        return;
      }
    }

    c->busy++;
    c->last_busy_time = 0;
    hostprintf(c, "ack #%d [%d]\n", cmd->line, cmd->code);
    cmd->ack_mbox = &c->ack_mbox;
    cmd->ack_evt = &c->ack_evt;
    printerPushCommand(
        (cmd->type & COMMANDTYPE_SyncAction) == COMMANDTYPE_SyncAction ?
            PRINTINGSOURCE_Host : PRINTINGSOURCE_None, cmd);
  }
  /*
   * TODO Checksum, line number
   */
}

static void send_report(HostContext* c) {
  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++)
  {
    uint8_t temp_id = machine.extruder.devices[i].temp_id;
    RadTempState s = temperatureGet(temp_id);
    uint8_t duty = outputGet(machine.temperature.devices[temp_id].heating_pwm_id);
    hostprintf(c, "T%d:%d /%d @%d:%d ", 
        i, (int)(s.pv+0.5), (int)(s.sv+0.5), i,
        // Firmware identifies itself as marlin,
        //   the duty is reported in the range of 0-127
        duty / 2);
  }
  for (uint8_t i = 0; i < machine.heated_bed.count; i++)
  {
    uint8_t temp_id = machine.heated_bed.devices[i].temp_id;
    RadTempState s = temperatureGet(temp_id);
    uint8_t duty = outputGet(machine.temperature.devices[temp_id].heating_pwm_id);
    hostprintf(c, "B:%d /%d B@:%d ", 
        (int)(s.pv+0.5), (int)(s.sv+0.5),
        // Firmware identifies itself as marlin,
        //   the duty is reported in the range of 0-127
        duty / 2);
  }
  hostprintf(c, "C:");
  PlannerVirtualPosition pos = stepperGetCurrentPosition();
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    hostprintf(c, " %c:%f", machine.kinematics.axes[i].name, pos.axes[i]);
  }
  hostprintf(c, "\n");
}

static msg_t threadDataHost(void* arg) {
  chRegSetThreadName("data-host");

  HostContext c;
  memset(&c, 0, sizeof(HostContext));
  c.channel = (BaseAsynchronousChannel*) arg;

  chMBInit(&c.ack_mbox, c.ack_mbox_buffer, COMMAND_BUFFER_SIZE + 1);
  chEvtInit(&c.ack_evt);
  chEvtRegisterMask(&c.ack_evt, &c.ack_listener, 1);
  chEvtRegisterMaskWithFlags(&c.channel->event,
      &c.host_listener, 2, CHN_CONNECTED | CHN_INPUT_AVAILABLE);
  gcodeResetParseContext(&c.parse_context);

  c.ptr = c.buf;
  while (1)
  {
    eventmask_t events = chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(200));
    if (events & 1) {
      PrinterCommand* ack_command;
      while (chMBFetch(&c.ack_mbox, (msg_t*) &ack_command, TIME_IMMEDIATE) == RDY_OK)
      {
        hostprintf(&c, "ok #%d [%d] {+}\n", ack_command->line, ack_command->code);
        printerFreeCommand(ack_command);
        c.busy--;
      }
      c.last_busy_time = chTimeNow();
    }
    if (events & 2) {
      flagsmask_t flags = chEvtGetAndClearFlags(&c.host_listener);
      if (flags & CHN_CONNECTED) {
        c.last_report_time = 0;
        c.last_busy_time = 0;
        printerRelease(PRINTINGSOURCE_Host);
        hostprintf(&c, "start\n");
      }
      if (flags & CHN_INPUT_AVAILABLE)
      {
        while (1)
        {
          msg_t i = chnGetTimeout(c.channel, TIME_IMMEDIATE);
          if (i < 0) break;

          // End of line
          if (i == '\n' || i == '\r')
          {
            gcodeResetParseContext(&c.parse_context);
            if (c.ptr == c.buf) continue;

            *c.ptr = '\0';
            process_new_line(&c);
            c.ptr = c.buf;
            continue;
          }

          if ((c.ptr - c.buf) >= COMMAND_LENGTH)
          {
            printerEstop(L_PRINTER_LINE_TOO_LONG);
            continue;
          }

          i = gcodeFilterCharacter(i, &c.parse_context);
          if (i)
            *(c.ptr++) = i;
        }
      }
    }
    if (c.last_report_time > 0&& chTimeNow() - c.last_report_time >= REPORT_INTERVAL)
    {
      send_report(&c);
      c.last_report_time += REPORT_INTERVAL;
    }
    if (c.last_busy_time > 0 && chTimeNow() - c.last_busy_time >= IDLE_INTERVAL)
    {
      printerRelease(PRINTINGSOURCE_Host);
      c.last_busy_time = 0;
    }
  }
  return 0;
}

void dataHostInit(void)
{
  chThdCreateStatic(waDataHost, sizeof(waDataHost), NORMALPRIO,
      threadDataHost, (void*)radboard.hmi.comm_channel);
}
