#include "ch.h"
#include "hal.h"

#include <stdio.h>

static void toggle_tx(void) {
  palTogglePad(IOPORT1, 21);
}

static void toggle_rx(void) {
  palTogglePad(IOPORT3, 30);
}

/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void txend1(UARTDriver *uartp) {
  (void)uartp;
}

static void txend2(UARTDriver *uartp) {
  (void)uartp;
}

static void rxchar(UARTDriver *uartp, uint16_t c)
{
  (void)uartp;
  (uint16_t)c;
  toggle_rx();
}

static UARTConfig uart_cfg = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  {PIOA, 9, PIO_MODE_A},
  {PIOA, 8, PIO_MODE_A},
  {0,0,0},
  {0,0,0},
  38400, 0, UART_MR_PAR_NO, 0, 0
};

#define buf_len 64

static uint8_t buf1[buf_len];
static uint8_t buf2[buf_len];
static char buf[10];

static msg_t ThreadUart(void *arg) {
  (void)arg;

  uint8_t slot = 0;

  uartStart(&UARTD1, &uart_cfg);

  uartStartReceive(&UARTD1, buf_len, buf1);

  while (TRUE) {
    chSysLock();
    size_t remaining = uartStopReceiveI(&UARTD1);
    slot = 1 - slot;
    uartStartReceiveI(&UARTD1, buf_len, slot ? buf2 : buf1);
    chSysUnlock();

    if (buf_len != remaining)
    {
      toggle_tx();
      buf[0] = (buf_len-remaining) / 10 + '0';
      buf[1] = (buf_len-remaining) % 10 + '0';
      buf[2] = ' ';

      while (!(UART->UART_SR & UART_SR_TXEMPTY));
      uartStartSend(&UARTD1, 3, buf);
      toggle_tx();
    } else {
      //chThdSleepMicroseconds(100);
    }
    //palTogglePad(IOPORT2, 27); // "L"
  }
  return 0;
}

/*
 * This is a periodic thread that does absolutely nothing except increasing
 * the seconds counter.
 */
static WORKING_AREA(waThreadUart, 128);

/*
 * Application entry point.
 */
int main(void) {

	halInit();
  /*
   * System initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  chSysInit();


  palSetPadMode(IOPORT1, 21, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(IOPORT3, 30, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(IOPORT2, 27, PAL_MODE_OUTPUT_PUSHPULL);

  toggle_tx();
  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThreadUart, sizeof(waThreadUart), NORMALPRIO, ThreadUart, NULL);

  chThdExit(0);
  return 0;
}

