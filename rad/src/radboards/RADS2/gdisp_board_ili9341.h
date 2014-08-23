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

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes RAD, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#include "hal.h"
#include "ch.h"

/*
 * Connect the ILI9341 screen to the LCD port
 * ILI3941     RADS2 LCD port
 *     VCC <-> 2
 *     GND <-> 1
 *      CS <-> 10
 *   RESET <-> 6
 *     D/C <-> 4
 *    MOSI <-> 7
 *     SCK <-> 9
 *     LED <-> 16
 */

static Semaphore sem;

static void txend_cb(UARTDriver *uartp) {
  (void) uartp;
  chSysLockFromIsr();
  chSemSignalI(&sem);
  chSysUnlockFromIsr();
}

static UARTConfig lcd_uart_cfg = {
  .tx_pin = {PIOA, 11, PIO_MODE_A},
  .sck_pin = {PIOA, 17, PIO_MODE_B},
  .speed = 10000000, // 10Mhz
  .mr = US_MR_PAR_NO | US_MR_CHRL_8_BIT | US_MR_USART_MODE_SPI_MASTER | US_MR_CLKO | US_MR_CPHA,
  .txend1_cb = txend_cb,
};

#define PORT_CS         IOPORT2 // PB25/RTS0
#define PIN_CS          25
#define PORT_CD         IOPORT1 // PA20/LCD-RS
#define PIN_CD          20
#define PORT_RST        IOPORT3 // PC19/LCD-EN
#define PIN_RST         19

#define PORT_BACKLIGHT  IOPORT3 // PC14/LCD-BL
#define PIN_BACKLIGHT   14

#define PORT            UARTD2

/**
 * @brief   Initialize the board for the display.
 * @notes	This board definition uses GPIO and assumes exclusive access to these GPIO pins
 * @notapi
 */
static inline void gdisp_lld_init_board(void) {
  uartStart(&PORT, &lcd_uart_cfg);
  PORT.reg.usart->US_CR = US_CR_FCS;
  // CS
  palSetPadMode(PORT_CS, PIN_CS, PAL_MODE_OUTPUT_PUSHPULL);
  // CD
  palSetPadMode(PORT_CD, PIN_CD, PAL_MODE_OUTPUT_PUSHPULL);
  // RST
  palSetPadMode(PORT_RST, PIN_RST, PAL_MODE_OUTPUT_PUSHPULL);
  // Backlight
  palSetPadMode(PORT_BACKLIGHT, PIN_BACKLIGHT, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(PORT_CS, PIN_CS);
  palSetPad(PORT_RST, PIN_RST);
  palClearPad(PORT_CD, PIN_CD);

  chSemInit(&sem, 0);
}

/**
 * @brief   Set or clear the lcd reset pin.
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 * @notapi
 */
static inline void gdisp_lld_reset_pin(bool_t state) {
  if (state)
    palClearPad(PORT_RST, PIN_RST);
  else
    palSetPad(PORT_RST, PIN_RST);
}

/**
 * @brief   Set the lcd back-light level.
 * @param[in] percent		0 to 100%
 * @notapi
 */
static inline void gdisp_lld_backlight(uint8_t percent) {
  if (percent > 0)
    palSetPad(PORT_BACKLIGHT, PIN_BACKLIGHT);
  else
    palClearPad(PORT_BACKLIGHT, PIN_BACKLIGHT);
}

/**
 * @brief   Send command to the display.
 * @param[in] cmd	The command to send *
 * @notapi
 */
static inline void gdisp_lld_write_command(uint8_t cmd) {
  palSetPad(PORT_CS, PIN_CS);
  palClearPad(PORT_CD, PIN_CD);
  palClearPad(PORT_CS, PIN_CS);
  uartStartSend(&PORT, 1, &cmd);
  chSemWait(&sem);
}

/**
 * @brief   Send data to the display.
 * @param[in] data	The data to send
 * @notapi
 */
static inline void gdisp_lld_write_data(uint8_t* data, size_t length) {
  palSetPad(PORT_CS, PIN_CS);
  palSetPad(PORT_CD, PIN_CD);
  palClearPad(PORT_CS, PIN_CS);
  uartStartSend(&PORT, length, data);
  chSemWait(&sem);
}

/** @} */

