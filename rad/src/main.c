#include "ch.h"
#include "hal.h"

#include <stdio.h>

static void toggle_tx(void) {
  palTogglePad(IOPORT1, 21);
}

static void toggle_rx(void) {
  palTogglePad(IOPORT3, 30);
}

static SerialConfig sd_cfg = {
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

  sdStart(&SD1, &sd_cfg);

  while (TRUE) {
    uint8_t b = chSequentialStreamGet(&SD1);
    toggle_tx();
    chSequentialStreamPut(&SD1, b);
    toggle_tx();
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

