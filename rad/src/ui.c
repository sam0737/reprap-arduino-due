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
 * @file    ui.c
 * @brief   User Interface
 *
 * @addtogroup UI
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "rad.h"

#include "ui.h"

#if HAL_USE_GFX
#include "gfx.h"
#endif

/*===========================================================================*/
/* Local variables and types.                                                */
/*===========================================================================*/

#define HAS_DISPLAY  (GFX_USE_TDISP || GFX_USE_GDISP)

#if HAS_DISPLAY
static WORKING_AREA(waDisplay, 2048);

typedef enum {
  DASHBOARD_Temp_Idle = 0,
  DASHBOARD_Temp_Heating = 1,
  DASHBOARD_Temp_Cooling = 2
} DashboardTempState;

typedef enum {
  DASHBOARD_Status = 0x01,
  DASHBOARD_StatusIcon = 0x02,
  DASHBOARD_TimeSpent = 0x04,
  DASHBOARD_Layer = 0x08,
  DASHBOARD_StorageState = 0x10,
  DASHBOARD_ZPos = 0x20,
  DASHBOARD_Progress = 0x40,
  DASHBOARD_Temperatures = 0x80,
  DASHBOARD_ActiveExtruder = 0x100,
  DASHBOARD_Line1 = 0x100,
  DASHBOARD_Line2 = 0x200,
  DASHBOARD_Reset = 0x400,
  DASHBOARD_All = 0xFFF
} UiParts;

typedef struct {
  DashboardTempState state;
  int16_t pv;
  int16_t sv;
} DashboardTempData;

typedef void (*display_fetcher_t)(void);
typedef void (*display_renderer_t)(void);

typedef struct {
  display_fetcher_t   fetcher;
  display_renderer_t  renderer;
  UiParts changed_parts;
  union
  {
    struct {
      systime_t subscreen_time;
      struct {
        char* text;
        struct {
          systime_t time;
          int16_t offset;
          int16_t length;
          uint8_t delay;
        } marquee;
      } status;
      struct {
        char filename[10];
        uint8_t percent;
      } progress;
      char* status_icon;
      uint16_t time_spent;
      uint16_t layer;
      RadStorageHost storage_state;
      float z_pos;

      DashboardTempData temps[RAD_NUMBER_TEMPERATURES];
      uint8_t active_extruder;
      uint8_t temps_screen;
    } dashboard;
  };
} UiState;

static UiState uiState;
#endif

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

#if HAS_DISPLAY

#include "ui/display_format.h"
#if GFX_USE_TDISP
#include "tdisp_lld_control.h"
#include "ui/display_tdisp.h"
#include "ui/dashboard_tdisp.h"
#endif
#if GFX_USE_GDISP
#include "ui/display_gdisp.h"
#include "ui/dashboard_gdisp.h"
#endif

#include "ui/dashboard_fetcher.h"

static msg_t threadDisplay(void *arg) {
  (void)arg;
  systime_t next;
  systime_t now;

  chRegSetThreadName("ui");
  display_ui_init();
  ui_dashboard_init();

  while (TRUE) {
    next = chTimeNow() + MS2ST(100); // 10 fps
    uiState.fetcher();
    uiState.renderer();
    if (uiState.changed_parts)
    {
#ifdef GDISP_CONTROL_LLD_FLUSH
  gdispControl(GDISP_CONTROL_LLD_FLUSH, NULL);
#endif
      uiState.changed_parts = 0;
    }
    now = chTimeNow();
    chThdSleep(now + MS2ST(1) > next ? MS2ST(1) : next - now);
  }
  return 0;
}
#endif

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void uiInit(void)
{
#if HAS_DISPLAY
  gfxInit();
  uiSetContrast(machine.ui.contrast);
  uiState.fetcher = ui_dashboard_fetcher;
  uiState.changed_parts = DASHBOARD_All;
  chThdCreateStatic(waDisplay, sizeof(waDisplay), NORMALPRIO - 2, threadDisplay, NULL);
#endif
}

void uiSetContrast(float contrast)
{
#if !HAS_DISPLAY
  (void) contrast;
#else
  display_lld_set_contrast(contrast);
#endif
}

/** @} */
