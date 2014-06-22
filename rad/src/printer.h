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
 * @file    printer.h
 * @brief   Printer control logic header
 *
 * @addtogroup PRINTER
 * @{
 */

#ifndef _RAD_PRINTER_H_
#define _RAD_PRINTER_H_

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#define COMMAND_BUFFER_SIZE 4

#include "data/datahost.h"
#include "data/datastorage.h"

typedef enum {
  PRINTERSTATE_Standby = 0x00,

  PRINTERSTATE_Printing = 0x10,

  PRINTERSTATE_Interrupting = 0x40,
  PRINTERSTATE_Interrupted = 0x41,

  PRINTERSTATE_Estopped = 0x80
} PrinterState;

typedef enum {
  PRINTINGSOURCE_None = 0,
  PRINTINGSOURCE_Storage = 1,
  PRINTINGSOURCE_Host = 2,
  PRINTINGSOURCE_Lcd = 3,
} PrintingSource;

#ifdef __cplusplus
extern "C" {
#endif
  void printerInit(void);

  uint8_t printerGetActiveExtruder(void);
  float printerGetFeedrateMultiplier(void);
  void printerSetFeedrateMultiplier(const float value);
  float printerGetFlowMultiplier(void);
  void printerSetFlowMultiplier(const float value);

  void printerRelease(const PrintingSource source);
  bool_t printerTryAcquire(const PrintingSource source);

  void printerInterrupt(const PrintingSource source);
  void printerResume(const PrintingSource source);

  void printerPushCommand(const PrintingSource source, const PrinterCommand* command);
  void printerFreeCommand(PrinterCommand* command);
  PrinterState printerGetStateI(void);
  PrinterState printerGetState(void);
  void printerSetStateI(PrinterState new_state);
  void printerSetState(PrinterState new_state);
  void printerTimeStart(void);
  void printerTimeStartI(void);
  void printerTimeStopI(void);
  int32_t printerTimeSpent(void);
  int32_t printerTimeSpentI(void);
  const char* printerIsEstopped(void);
  void printerEstop(const char* message);
  void printerEstopClear(void);
  void printerSetMessage(const char* message);
  const char* printerGetMessage(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_PRINTER_H_ */

/** @} */
