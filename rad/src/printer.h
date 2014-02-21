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

typedef enum {
  PRINTERSTATE_Standby = 0x00,

  PRINTERSTATE_Printing = 0x10,
  PRINTERSTATE_Heating = 0x11,

  PRINTERSTATE_Interrupting = 0x40,
  PRINTERSTATE_Interrupted = 0x41,

  PRINTERSTATE_Estopped = 0x80
} PrinterState;

typedef enum {
  PRINTINGSOURCE_SD = 0,
  PRINTINGSOURCE_Host = 1,
} PrintingSource;

extern char* printer_estop_message;

#ifdef __cplusplus
extern "C" {
#endif
  void printerInit(void);
  uint8_t printerGetActiveExtruder(void);
  void printerAddLine(const char* line);
  PrinterState printerGetState(void);
  bool_t printerIsEstopped(void);
  void printerEstop(char* message);
  void printerEstopClear(void);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_PRINTER_H_ */

/** @} */
