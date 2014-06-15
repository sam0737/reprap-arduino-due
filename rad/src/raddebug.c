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
#include "chprintf.h"
#include "shell.h"

#if HAL_USE_GFX
#include "gfx.h"
#endif
#if RAD_STORAGE
#include "ff.h"
#endif

#include "raddebug.h"
#include <stdlib.h>

/*===========================================================================*/
/* Shell entry points.                                                       */
/*===========================================================================*/

#include "debug/test_planner.h"
#include "debug/benchmark.h"

volatile int32_t debug_value[24];

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;
  (void)argc;
  (void)argv;
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
  chprintf(chp, "sizeof PlannerOutputBlock: %u bytes\r\n", sizeof(PlannerOutputBlock));
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  chprintf(chp, "      name     addr prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%10s %.8lx %4lu %4lu %9s %lu\r\n",
#if CH_USE_REGISTRY
            tp->p_name,
#else
            "",
#endif
            (uint32_t)tp,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_dump(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argv;
  (void)argc;

  storageDumpConfig();
}

static void cmd_erase(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

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
    chThdSleepMilliseconds(750);
    beeperPlay(&tuneFinished);
    chThdSleepMilliseconds(750);
    beeperPlay(&tuneOk);
    chThdSleepMilliseconds(750);
    beeperPlay(&tuneScroll);
    chThdSleepMilliseconds(750);
    beeperPlay(&tuneWarning);
    return;
  } else if (argc == 1) {
    tone = atoi(argv[0]);
    tuneDebug.notes[0].tone = tone;
    beeperPlay(&tuneDebug);
  } else {
    chprintf(chp, "Usage: beep [freq]\r\n");
  }
}

static void cmd_status(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  for (uint8_t i = 0; i < 24; i++) {
    if (i % 4 == 0 && i > 0) chprintf(chp, "\r\n");
    if (i % 8 == 0 && i > 0) chprintf(chp, "\r\n");
    chprintf(chp, "  %.8x", debug_value[i]);
  }
  chprintf(chp, "\r\n");

  chprintf(chp, "Temperatures:\r\n");
  RadTempState temp;
  uint8_t temp_id;
  for (uint8_t i = 0; i < RAD_NUMBER_EXTRUDERS; i++) {
    temp = temperatureGet(temp_id = machine.extruder.devices[i].temp_id);
    chprintf(chp, "  Extruder %d: PV %6.2f [%.4d] >> SV %.2f (@%3d)\r\n", i, temp.pv, temp.raw, temp.sv,
        outputGet(machine.temperature.devices[temp_id].heating_pwm_id));
  }
  for (uint8_t i = 0; i < machine.heated_bed.count; i++) {
    temp = temperatureGet(temp_id = machine.heated_bed.devices[i].temp_id);
    chprintf(chp, "  Bed      %d: PV %6.2f [%.4d] >> SV %.2f (@%3d)\r\n", i, temp.pv, temp.raw, temp.sv,
        outputGet(machine.temperature.devices[temp_id].heating_pwm_id));
  }
  for (uint8_t i = 0; i < machine.temp_monitor.count; i++) {
    temp = temperatureGet(temp_id = machine.temp_monitor.devices[i].temp_id);
    chprintf(chp, "  Monitor  %d: PV %6.2f [%.4d] >> SV %.2f (@%3d)\r\n", i, temp.pv, temp.raw, temp.sv,
        outputGet(machine.temperature.devices[temp_id].heating_pwm_id));
  }

  RadJointsState joints_state = stepperGetJointsState();
  chprintf(chp, "Joints:\r\n");
  for (uint8_t i = 0; i < RAD_NUMBER_JOINTS; i++) {
    RadJointState* js = &joints_state.joints[i];
    chprintf(chp, "  %d Pos: %8.2f  Limit: %c%c  Stopped: %d  Homed: %d\r\n",
        i,
        js->pos,
        ((js->limit_state & LIMIT_MinHit) != 0) ? '-' : (js->limit_state == LIMIT_Normal ? '*' : ' '),
        ((js->limit_state & LIMIT_MaxHit) != 0) ? '+' : ' ',
        js->stopped, js->homed);
  }

  PlannerVirtualPosition virtual = stepperGetCurrentPosition();
  chprintf(chp, "Position:\r\n");
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++) {
    chprintf(chp, "  %c: %-4.2f ",
        (char)machine.kinematics.axes[i].name,
        virtual.axes[i]);
  }

  chprintf(chp, "\r\nStorage: %d. Queue: %d\r\n", storageGetHostState(), plannerQueueGetLength(&queueMain));
  const char* message = printerGetMessage();
  chprintf(chp, "Status: %s\r\n", message ? message : "<NULL>");
}

static void cmd_out(BaseSequentialStream *chp, int argc, char *argv[]) {
    int ch;
    int duty;
    char* end;

    if (argc == 2) {
      do {
        ch = strtol(argv[0], &end, 10);
        if (argv[0] == end) break;
        duty = strtol(argv[1], &end, 10);
        if (argv[1] == end) break;

        outputSet(ch, (uint8_t) duty);
        return;
      } while(0);
    }
    chprintf(chp, "Usage: out [ch] [0-255]\r\n");
}

static void cmd_temp(BaseSequentialStream *chp, int argc, char *argv[]) {
    int ch;
    int temp;
    char* end;

    if (argc == 2) {
      do {
        ch = strtol(argv[0], &end, 10);
        if (argv[0] == end) break;
        temp = strtol(argv[1], &end, 10);
        if (argv[1] == end) break;

        temperatureSet(ch, temp);
        return;
      } while(0);
    }
    chprintf(chp, "Usage: temp [ch] [temperature]\r\n");
}

static void cmd_tune_temp(BaseSequentialStream *chp, int argc, char *argv[]) {
    int ch;
    int target;
    char* end;

    if (argc == 2) {
      do {
        ch = strtol(argv[0], &end, 10);
        if (argv[0] == end) break;
        target = strtol(argv[1], &end, 10);
        if (argv[1] == end) break;

        temperatureAutoTune(ch, target, 5);
        return;
      } while(0);
    }
    chprintf(chp, "Usage: tune_temp [ch] [temperature]\r\n");
}

/*
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
*/

//char path[1024];
//static void cmd_sd(BaseSequentialStream *chp, int argc, char *argv[]) {
  //(void)argv;
    //(void)argc;
    //FRESULT err;

    /*
    if (mmcConnect(&MMCD1)) {
      chprintf(chp, "Connection failed\r\n");
      return;
    }*/

    /*
    chprintf(chp, "Mounting...\r\n");
    err = f_mount(0, &MMC_FS);
    if (err != FR_OK) {
      //mmcDisconnect(&MMCD1);
      chprintf(chp, "Mount failed\r\n");
      return;
    }

    chprintf(chp, "Capacity: %d\r\n", MMCD1.capacity);

    path[0] = 0;
    scan_files(chp, path);
    */
//}

static void cmd_mount(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  storageUsbMount();
}

static void cmd_unmount(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  storageUsbUnmount();
}

static void cmd_homing(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  //printerAddLine("G28");
}

static void cmd_stop(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  PlannerJointMovement m;
  memset(&m, 0, sizeof(PlannerJointMovement));
  plannerSetJointVelocity(&m);
}

static void cmd_vel(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  PlannerJointMovement m;
  memset(&m, 0, sizeof(PlannerJointMovement));
  uint8_t k = 0;
  m.stop_on_limit_changes = TRUE;
  m.rapid = TRUE;
  for (uint8_t i = 0; k < argc && i < RAD_NUMBER_JOINTS; i++, k++) {
    m.joints[i] = strtof(argv[k], NULL);
  }
  for (uint8_t i = 0; k < argc && i < RAD_NUMBER_EXTRUDERS; i++, k++) {
    m.extruders[i] = strtof(argv[k], NULL);
  }
  plannerSetJointVelocity(&m);
}

static void cmd_estop(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  printerEstop(L_DEBUG_STOPPED_BY_SHELL);
}

static void cmd_clear(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  printerEstopClear();
}

static void cmd_reset(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;

  debugReset();
}

static void cmd_contrast(BaseSequentialStream *chp, int argc, char *argv[]) {
  int contrast;

  if (argc < 1) {
    chprintf(chp, "Usage: contrast <0-100>\r\n");
    return;
  }

  contrast = atoi(argv[0]);
  if (contrast > 100) contrast = 100;
  if (contrast < 0) contrast = 0;
  uiSetContrast(contrast / 100.0);
}

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

static WORKING_AREA(waHeartbeat, 128);
static WORKING_AREA(waShellMonitor, 512);

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"erase", cmd_erase},
  {"dump", cmd_dump},
  {"power", cmd_power},
  {"beep", cmd_beep},
  {"status", cmd_status},
  {"out", cmd_out},
  {"temp", cmd_temp},
  {"tune_temp", cmd_tune_temp},
  {"mount", cmd_mount},
  {"unmount", cmd_unmount},
  {"homing", cmd_homing},
  {"stop", cmd_stop},
  {"vel", cmd_vel},
  {"estop", cmd_estop},
  {"clear", cmd_clear},
  {"reset", cmd_reset},
  {"contrast", cmd_contrast},
  {"benchmark", cmd_benchmark},
#if RAD_TEST
  {"t", cmd_test_planner},
  {"test_planner", cmd_test_planner},
#endif
  {NULL, NULL}
};

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

static msg_t threadHeartbeat(void *arg) {
  (void)arg;
  chRegSetThreadName("heartbeat");

#if HAL_USE_USB
  palSetPinMode(radboard.debug.heartbeat_led, PAL_MODE_OUTPUT_PUSHPULL);
  while (TRUE) {
    systime_t time = USBD1.state == USB_ACTIVE ? 250 : 2500;
    chThdSleepMilliseconds(time);
    palTogglePad(radboard.debug.heartbeat_led.port, radboard.debug.heartbeat_led.pin);
  }
#endif
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
    chThdCreateStatic(waHeartbeat, sizeof(waHeartbeat), LOWPRIO + 10, threadHeartbeat, NULL);
  }

  if (radboard.debug.channel != NULL)
  {
    shellInit();
    chThdCreateStatic(waShellMonitor, sizeof(waShellMonitor), LOWPRIO, threadShellMonitor, NULL);
  }
}

void debugErase(void)
{
  if (radboard.debug.erase != NULL)
    radboard.debug.erase();
}

void debugReset(void)
{
  if (radboard.debug.software_reset != NULL)
    radboard.debug.software_reset();
}

/** @} */
