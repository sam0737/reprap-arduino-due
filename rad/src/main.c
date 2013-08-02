#include "ch.h"
#include "hal.h"

//#include <stdio.h>

#include "chprintf.h"
#include "rad.h"

void toggle_tx(void) {
  pexTogglePad(IOPORT1, 21);
}

void toggle_rx(void) {
  pexTogglePad(IOPORT3, 30);
}

void toggle_L(void) {
  pexTogglePad(IOPORT2, 27);
}

void debug(const char* message) {
  chprintf((BaseSequentialStream *)&SD1, message);
}


static SerialConfig sd_cfg = {
  {PIOA, 9, PIO_MODE_A},
  {PIOA, 8, PIO_MODE_A},
  {0,0,0},
  {0,0,0},
  115200, 0, UART_MR_PAR_NO, 0, 0
};

uint8_t data[] = "*=A2345678901234567890123456789012345678901234567890B2345678901234567890123456789012345678901234567890C2345678901234567890123456789012345678901234567890D2345678901234567890123456789012345678901234567890E2345678901234567890123456789012345678901234567890F2345678901234567890123456789012345678901234567890G2345678901234567890123456789012345678901234567890H2345678901234567890123456789012345678901234567890I2345678901234567890123456789012345678901234567890J23456789012345678901234567890123456789012345678901234567890=|"
    "*=A2345678901234567890123456789012345678901234567890B2345678901234567890123456789012345678901234567890C2345678901234567890123456789012345678901234567890D2345678901234567890123456789012345678901234567890E2345678901234567890123456789012345678901234567890F2345678901234567890123456789012345678901234567890G2345678901234567890123456789012345678901234567890H2345678901234567890123456789012345678901234567890I2345678901234567890123456789012345678901234567890J23456789012345678901234567890123456789012345678901234567890=|";
static msg_t ThreadUart(void *arg) {
  (void)arg;
  while (TRUE) {
      uint8_t b = chSequentialStreamGet(&SD1);
      chSequentialStreamPut(&SD1, b);
    }
  return 0;
}

static msg_t ThreadUart1(void *arg) {
  (void)arg;
  chRegSetThreadName("prog uart");

  while (TRUE) {
    while (USBD1.state != USB_ACTIVE) {
      chThdSleepMilliseconds(50);
    }
    uint8_t b = chSequentialStreamGet(&SDU_DATA);
    chThdSleepMilliseconds(100);
    chprintf((BaseSequentialStream *)&SD1, ">>%.2x\r\n", b);
    chSequentialStreamPut(&SDU_DATA, b);
  }
  return 0;
}

static WORKING_AREA(waThreadUart, 1024);
static WORKING_AREA(waThreadUart1, 1024);

/*
 * Application entry point.
 */
int main(void) {
  Thread* t;

	halInit();
  /*
   * System initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  chSysInit();

  sdStart(&SD1, &sd_cfg);

  radInit();

  palSetPadMode(IOPORT1, 21, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(IOPORT3, 30, PAL_MODE_OUTPUT_PUSHPULL);

  //chprintf((BaseSequentialStream *)&SD1, "!!! MAIN !!!\r\n");
  //chThdCreateStatic(waThreadUart, sizeof(waThreadUart), NORMALPRIO, ThreadUart, NULL);

  //return 0;

  t = chThdCreateStatic(waThreadUart1, sizeof(waThreadUart1), NORMALPRIO, ThreadUart1, NULL);

  chThdExit(0);
  return 0;
}

