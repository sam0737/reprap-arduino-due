#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "rad.h"

/*
 * Application entry point.
 */
int main(void) {
  //Thread* t;
	halInit();
  /*
   * System initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  chSysInit();

  radInit();

  //palSetPadMode(IOPORT1, 21, PAL_MODE_OUTPUT_PUSHPULL);
  //palSetPadMode(IOPORT3, 30, PAL_MODE_OUTPUT_PUSHPULL);

  //chprintf((BaseSequentialStream *)&SD1, "!!! MAIN !!!\r\n");
  //chThdCreateStatic(waThreadUart, sizeof(waThreadUart), NORMALPRIO, ThreadUart, NULL);

  //return 0;

  //t = chThdCreateStatic(waThreadUart1, sizeof(waThreadUart1), NORMALPRIO, ThreadUart1, NULL);

  chThdExit(0);
  return 0;
}

