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
  US_MR_USART_MODE_SPI_MASTER | US_MR_CLKO | US_MR_CPOL,
  .txend1_cb = txend_cb,
};

#define PORT_CS         IOPORT1
#define PIN_CS          18
#define PORT_A0         IOPORT3
#define PIN_A0          13
#define PORT_RST        IOPORT3
#define PIN_RST         15

#define PORT            UARTD2

/**
 * @brief   Initialize the board for the display.
 * @notes	This board definition uses GPIO and assumes exclusive access to these GPIO pins
 * @notapi
 */
static inline void init_board(void) {
  uartStart(&PORT, &lcd_uart_cfg);
  PORT.reg.usart->US_CR = US_CR_FCS;
  // A0
  palSetPadMode(PORT_A0, PIN_A0, PAL_MODE_OUTPUT_PUSHPULL);
  // CS
  palSetPadMode(PORT_CS, PIN_CS, PAL_MODE_OUTPUT_PUSHPULL);
  // RST
  palSetPadMode(PORT_RST, PIN_RST, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(PORT_CS, PIN_CS);
  palSetPad(PORT_RST, PIN_RST);
  palClearPad(PORT_A0, PIN_A0);

  chSemInit(&sem, 1);
}

/**
 * @brief   Set or clear the lcd reset pin.
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 * @notapi
 */
static inline void setpin_reset(bool_t state) {
  palClearPad(PORT_CS, PIN_CS);
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
static inline void set_backlight(uint8_t percent) {
  (void) percent;
}

/**
 * @brief   Take exclusive control of the bus
 * @notapi
 */
static inline void acquire_bus(void) {

}

/**
 * @brief   Release exclusive control of the bus
 * @notapi
 */
static inline void release_bus(void) {

}

static uint8_t cmd_budfer;
/**
 * @brief   Send command to the display.
 * @param[in] cmd	The command to send *
 * @notapi
 */
static inline void write_cmd(uint8_t cmd) {
  (void) cmd;

  chSemWait(&sem);

  cmd_budfer = cmd;
  palClearPad(PORT_A0, PIN_A0);
  uartStartSend(&PORT, 1, &cmd_budfer);
}

/**
 * @brief   Send data to the display.
 * @param[in] data	The data to send
 * @notapi
 */
static inline void write_data(uint8_t* data, uint16_t length) {
  (void) data;
  (void) length;

  chSemWait(&sem);

  palSetPad(PORT_A0, PIN_A0);
  uartStartSend(&PORT, length, data);
}

/** @} */

