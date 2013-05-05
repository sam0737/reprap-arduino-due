#include "ch.h"
#include "hal.h"

#include <stdio.h>

#include "chprintf.h"
#include "usbcfg.h"

/* Virtual serial port over USB.*/
static SerialUSBDriver SDU1;

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

static msg_t ThreadUart2(void *arg) {
  (void)arg;

  while (TRUE) {
    while (serusbcfg.usbp->state != USB_ACTIVE) {
      chThdSleepMilliseconds(50);
    }
    uint8_t b = chSequentialStreamGet(&SDU1);
    //chprintf((BaseSequentialStream *)&SD1, "/// GOT: %.2x\r\n", b);
    chSequentialStreamWrite(&SDU1, "apachemeowka", 12);
    chSequentialStreamPut(&SDU1, b);
  }
  return 0;
}

static msg_t Heartbeat(void *arg) {
  (void)arg;

  while (TRUE) {
    systime_t time = serusbcfg.usbp->state == USB_ACTIVE ? 250 : 500;
    chThdSleepMilliseconds(time);
    toggle_L();
  }
  return 0;
}

static WORKING_AREA(waThreadUart, 1024);
static WORKING_AREA(waThreadUart2, 1024);
static WORKING_AREA(waHeartbeat, 1024);

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
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  usbStart(serusbcfg.usbp, &usbcfg);
  chThdSleepMilliseconds(500);
  usbConnectBus(serusbcfg.usbp);

  chThdCreateStatic(waThreadUart2, sizeof(waThreadUart2), NORMALPRIO, ThreadUart2, NULL);
  chThdExit(0);
  return 0;
}

