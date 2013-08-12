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
 * @file    raddebug.c
 * @brief   debug utilities
 *
 * @addtogroup RADDEBUG
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"
#include "ff.h"

#include "chprintf.h"
#include "shell.h"

#include "raddebug.h"

FATFS MMC_FS;

/*===========================================================================*/
/* Shell entry points.                                                       */
/*===========================================================================*/

volatile int32_t debug_value[24];

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
  chprintf(chp, "      name     addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%10s %.8lx %.8lx %4lu %4lu %9s %lu\r\n",
#if CH_USE_REGISTRY
            tp->p_name,
#else
            "",
#endif
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_dump(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  (void)argc;
  uint8_t i;

  for (i = 0; i < 24; i++) {
    if (i % 8 == 0 && i > 0) chprintf(chp, "\r\n");
    chprintf(chp, " %.8x\r\n", debug_value[i]);
  }
  chprintf(chp, "\r\n");
}

static void cmd_erase(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: erase\r\n");
    return;
  }
  debugErase();
  chprintf(chp, "Erase is not supported on " RADBOARD_NAME "\r\n");
}

static void cmd_power(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
  if (argc != 1) {
    chprintf(chp, "Usage: power [on|off]\r\n");
    return;
  }
  if (strcmp(argv[0],"on") == 0)
  {
    powerPsuOn();
  } else {
    powerPsuOff();
  }
}

static BeeperTune tuneDebug = { .notes = (BeeperNote[]) {
  {0, 100}, {0, 0}
} };

static void cmd_beep(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    int tone;
    if (argc == 0) {
      beeperPlay(&tuneStartup);
      return;
    } else if (argc == 1) {
      if (sscanf(argv[0], "%d", &tone) == 1) {
        tuneDebug.notes[0].tone = tone;
        beeperPlay(&tuneDebug);
      }
    }
    chprintf(chp, "Usage: beep [freq]\r\n");
}

static void cmd_status(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  chprintf(chp, "Temperatures:\r\n");
  for (uint8_t i = 0; i < machine.extruder.count; i++) {
    RadTemp *cht = machine.extruder.devices[i].temp;
    chprintf(chp, "  Extruder %d: PV %.2f [%.4d] >> SV %.2f\r\n", i, cht->state.pv, cht->state.raw, cht->state.sv);
  }
  for (uint8_t i = 0; i < machine.heated_bed.count; i++) {
    RadTemp *cht = machine.heated_bed.devices[i].temp;
    chprintf(chp, "  Bed      %d: PV %.2f [%.4d] >> SV %.2f\r\n", i, cht->state.pv, cht->state.raw, cht->state.sv);
  }
  for (uint8_t i = 0; i < machine.temp_monitor.count; i++) {
    RadTemp *cht = machine.temp_monitor.devices[i];
    chprintf(chp, "  Monitor  %d: PV %.2f [%.4d] >> SV %.2f\r\n", i, cht->state.pv, cht->state.raw, cht->state.sv);
  }

  chprintf(chp, "Joints:\r\n");
  for (uint8_t i = 0; i < machine.kinematics.joint_count; i++) {
    chSysLock();
    RadJointState s = machine.kinematics.joints[i].state;
    chSysUnlock();
    chprintf(chp, "  %d Pos: %-4.2f  Limit: %c%c  Stopped: %d  Homed: %d\r\n",
        i,
        s.pos,
        ((s.limit_state & LIMIT_MinHit) != 0) ? '-' : (s.limit_state == LIMIT_Normal ? '*' : ' '),
        ((s.limit_state & LIMIT_MaxHit) != 0) ? '+' : ' ',
        s.stopped, s.homed);
  }

  chprintf(chp, "E Stopped:\r\n  %s\r\n", printer_estop_message ? printer_estop_message : "None");
}

static void cmd_out(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    int ch;
    int duty;

    if (argc == 2) {
      if (sscanf(argv[0], "%d", &ch) == 1 && sscanf(argv[1], "%d", &duty) == 1) {
        if (ch >= 0 && ch < radboard.output.count) {
          outputSet(&radboard.output.channels[ch], (uint8_t) duty);
          return;
        }
      }
    }
    chprintf(chp, "Usage: out [ch] [0-255]\r\n");
}

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int i;
  char *fn;

#if _USE_LFN
  fno.lfname = 0;
  fno.lfsize = 0;
#endif
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;
      if (fno.fname[0] == '.')
        continue;
      fn = fno.fname;
      if (fno.fattrib & AM_DIR) {
        path[i++] = '/';
        strcpy(&path[i], fn);
        res = scan_files(chp, path);
        if (res != FR_OK)
          break;
        path[--i] = 0;
      }
      else {
        chprintf(chp, "%s/%s\r\n", path, fn);
      }
    }
  }
  return res;
}

PARTITION VolToPart[] = { {0, 0} };
char path[1024];
static void cmd_sd(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    (void)argc;
    FRESULT err;

    if (mmcConnect(&MMCD1)) {
      chprintf(chp, "Connection failed\r\n");
      return;
    }

    chprintf(chp, "Mounting...\r\n");
    err = f_mount(0, &MMC_FS);
    if (err != FR_OK) {
      mmcDisconnect(&MMCD1);
      chprintf(chp, "Mount failed\r\n");
      return;
    }

    chprintf(chp, "Capacity: %d\r\n", MMCD1.capacity);

    path[0] = 0;
    scan_files(chp, path);
}

static void cmd_insert(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  msdReady(&UMSD, (BaseBlockDevice *)&MMCD1);
}

static void cmd_eject(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  msdEject(&UMSD);
}

static void cmd_homing(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  printerAddLine("G29");
}

static void cmd_estop(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  printerEstop("Stopped by shell");
}

static void cmd_clear(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  printerEstopClear();
}

static void p1(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  PlannerJointMovement v = {
    .joints = {10, -4, 0},
    .extruders = {5, 0},
    .rapid = 1
  };
  plannerSetJointVelocity(&v);
}

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waHeartbeat, 128);
static WORKING_AREA(waShellMonitor, 128);

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"erase", cmd_erase},
  {"dump", cmd_dump},
  {"power", cmd_power},
  {"beep", cmd_beep},
  {"status", cmd_status},
  {"out", cmd_out},
  {"sd", cmd_sd},
  {"insert", cmd_insert},
  {"eject", cmd_eject},
  {"homing", cmd_homing},
  {"estop", cmd_estop},
  {"clear", cmd_clear},
  {NULL, NULL}
};

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static msg_t threadHeartbeat(void *arg) {
  (void)arg;
  chRegSetThreadName("heartbeat");

  palSetPinMode(radboard.debug.heartbeat_led, PAL_MODE_OUTPUT_PUSHPULL);
  while (TRUE) {
    systime_t time = USBD1.state == USB_ACTIVE ? 250 : 500;
    chThdSleepMilliseconds(time);
    palTogglePad(radboard.debug.heartbeat_led.port, radboard.debug.heartbeat_led.pin);
  }
  return 0;
}

static msg_t threadShellMonitor(void *arg) {
  (void)arg;
  chRegSetThreadName("shell mon");

  Thread *tShell = NULL;

  ShellConfig shell_cfg = {
    (BaseSequentialStream*) radboard.debug.channel,
    commands
  };

  while (1) {
    if (!tShell) {
      tShell = shellCreate(&shell_cfg, THD_WA_SIZE(1024), NORMALPRIO);
    }
    else if (chThdTerminated(tShell)) {
      chThdRelease(tShell);    /* Recovers memory of the previous shell. */
      tShell = NULL;           /* Triggers spawning of a new shell.      */
    }
    chThdSleepMilliseconds(500);
  }
  chThdExit(0);
  return 0;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void debugInit(void)
{
  if (palHasPin(radboard.debug.heartbeat_led)) {
    chThdCreateStatic(waHeartbeat, sizeof(waHeartbeat), NORMALPRIO, threadHeartbeat, NULL);
  }

  if (radboard.debug.channel != NULL)
  {
    shellInit();
    chThdCreateStatic(waShellMonitor, sizeof(waShellMonitor), NORMALPRIO, threadShellMonitor, NULL);
  }
}

void debugErase(void)
{
  if (radboard.debug.erase_callback != NULL)
    radboard.debug.erase_callback();
}

/** @} */