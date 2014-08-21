/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "hal.h"
#include "ch.h"

static Semaphore sem;

static void txend_cb(UARTDriver *uartp) {
  (void) uartp;
  chSysLockFromIsr();
  chSemSignalI(&sem);
  chSysUnlockFromIsr();
}

static UARTConfig lcd_uart_cfg = {
  .tx_pin = {PIOA, 11, PIO_MODE_A},
  .rx_pin = {PIOA, 17, PIO_MODE_B}, // SCK Pin
  .speed = 10000000, // 10Mhz
  .mr = US_MR_PAR_NO | US_MR_CHRL_8_BIT |
  US_MR_USART_MODE_SPI_MASTER | US_MR_CLKO | US_MR_CPHA,
  .txend1_cb = txend_cb,
};

#define PORT_CS         IOPORT3 // C24
#define PIN_CS          24
#define PORT_CD         IOPORT3 // C25
#define PIN_CD          25
#define PORT_RST        IOPORT3 // C26
#define PIN_RST         26

#define PORT            UARTD2

/**
 * @brief   Initialize the board for the display.
 * @notes	This board definition uses GPIO and assumes exclusive access to these GPIO pins
 * @notapi
 */
static inline void gdisp_lld_init_board(void) {
  uartStart(&PORT, &lcd_uart_cfg);
  PORT.reg.usart->US_CR = US_CR_FCS;
  // A0
  palSetPadMode(PORT_CD, PIN_CD, PAL_MODE_OUTPUT_PUSHPULL);
  // CS
  palSetPadMode(PORT_CS, PIN_CS, PAL_MODE_OUTPUT_PUSHPULL);
  // RST
  palSetPadMode(PORT_RST, PIN_RST, PAL_MODE_OUTPUT_PUSHPULL);
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
  (void) percent;
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

