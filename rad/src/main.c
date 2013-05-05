#include "ch.h"
#include "hal.h"

#include <stdio.h>

#include "chprintf.h"
#include "usbcfg.h"
#include "shell.h"

/* Virtual serial port over USB.*/
SerialUSBDriver SDU_SHELL;
SerialUSBDriver SDU_DATA;

Thread *shelltp = NULL;

void toggle_tx(void) {
  palTogglePad(IOPORT1, 21);
}

void toggle_rx(void) {
  palTogglePad(IOPORT3, 30);
}

void toggle_L(void) {
  palTogglePad(IOPORT2, 27);
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
    if (b == 'a')
    {
      usbPrepareTransmit(&USBD1, 2, data, 30);
      debug("PDONE\r\n");
      chSysLock();
      usbStartTransmitI(&USBD1, 2);
      chSysUnlock();
    } else if (b == 'd') {
      chprintf((BaseSequentialStream *)&SD1, "es%.8x msk%.8x dc%.8x dms%.8x \r\n",
          UOTGHS->UOTGHS_DEVEPTISR[2],
          UOTGHS->UOTGHS_DEVEPTIMR[2],
          UOTGHS->UOTGHS_DEVDMA[1].UOTGHS_DEVDMACONTROL,
          UOTGHS->UOTGHS_DEVDMA[1].UOTGHS_DEVDMASTATUS);
    }
    chSequentialStreamPut(&SD1, b);
  }
  return 0;
}

static msg_t ThreadUart1(void *arg) {
  (void)arg;

  while (TRUE) {
    while (USBD1.state != USB_ACTIVE) {
      chThdSleepMilliseconds(50);
    }
    uint8_t b = chSequentialStreamGet(&SDU_DATA);
    chSequentialStreamPut(&SDU_DATA, b);
  }
  return 0;
}

static msg_t Heartbeat(void *arg) {
  (void)arg;

  while (TRUE) {
    systime_t time = USBD1.state == USB_ACTIVE ? 250 : 500;
    chThdSleepMilliseconds(time);
    toggle_L();
  }
  return 0;
}

static WORKING_AREA(waThreadUart, 1024);
static WORKING_AREA(waThreadUart1, 1024);
static WORKING_AREA(waHeartbeat, 1024);

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {NULL, NULL}
};

static const ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SDU_SHELL,
  commands
};

/*
 * Application entry point.
 */
int main(void) {
  // Disable Watchdog
  WDT->WDT_MR = WDT_MR_WDDIS;

	halInit();
  /*
   * System initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  chSysInit();
  shellInit();

  palSetPadMode(IOPORT1, 21, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(IOPORT3, 30, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(IOPORT2, 27, PAL_MODE_OUTPUT_PUSHPULL);

  sdStart(&SD1, &sd_cfg);
  chThdCreateStatic(waThreadUart, sizeof(waThreadUart), NORMALPRIO, ThreadUart, NULL);
  chThdCreateStatic(waHeartbeat, sizeof(waHeartbeat), NORMALPRIO, Heartbeat, NULL);

  chprintf((BaseSequentialStream *)&SD1, "!!! MAIN %d !!!\r\n", (RSTC->RSTC_SR & RSTC_SR_RSTTYP_Msk)>>RSTC_SR_RSTTYP_Pos);
  //chThdExit(0);
  //return 0;

  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU_SHELL);
  sduStart(&SDU_SHELL, &serusb_shellcfg);
  sduObjectInit(&SDU_DATA);
  sduStart(&SDU_DATA, &serusb_datacfg);
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(&USBD1);
  usbStart(&USBD1, &usbcfg);
  chThdSleepMilliseconds(500);
  usbConnectBus(&USBD1);

  chThdCreateStatic(waThreadUart1, sizeof(waThreadUart1), NORMALPRIO, ThreadUart1, NULL);

  while (1) {
    if (!shelltp)
      shelltp = shellCreate(&shell_cfg, THD_WA_SIZE(1024), NORMALPRIO);
    else if (chThdTerminated(shelltp)) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell. */
      shelltp = NULL;           /* Triggers spawning of a new shell.      */
    }
    chThdSleepMilliseconds(500);
  }
  chThdExit(0);
  return 0;
}

