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
static WORKING_AREA(waDisplay, 1024);

#define ENABLE_INPUT(TYPE) \
  do { \
    if (machine.ui.TYPE.enabled) \
      inputEnable(machine.ui.TYPE.input_id); \
  } while (0)

#define FLUSH_INPUT(TYPE) \
  do { \
    if (machine.ui.TYPE.enabled) \
      inputGet(machine.ui.TYPE.input_id); \
  } while (0)

#define GET_INPUT(TYPE) \
  inputGet(machine.ui.TYPE.enabled ? machine.ui.TYPE.input_id : 255)

typedef enum {
  DASHBOARD_Temp_Idle = 0,
  DASHBOARD_Temp_Heating = 1,
  DASHBOARD_Temp_Cooling = 2
} DashboardTempState;

typedef enum {
  DASHBOARD_Status = 0x01,
  DASHBOARD_TimeSpent = 0x02,
  DASHBOARD_Layer = 0x04,
  DASHBOARD_StorageState = 0x08,
  DASHBOARD_Pos = 0x10,
  DASHBOARD_Source = 0x20,
  DASHBOARD_Progress = 0x40,
  DASHBOARD_Temperatures = 0x80,
  DASHBOARD_TemperaturesChangeScreen = 0x100,
  DASHBOARD_ActiveExtruder = 0x200,
  DASHBOARD_Line1 = 0x1000,
  DASHBOARD_Line2 = 0x2000,
  DASHBOARD_Reset = 0x4000,
  MENU_Changed = 0x800,
  UI_PARTS_All = 0xFFFF
} UiParts;

typedef struct {
  DashboardTempState state;
  int16_t pv;
  int16_t sv;
} DashboardTempData;

typedef struct {
  float pos;
  bool_t homed;
  RadLimitState limit_state;
} DashboardAxis;

typedef void (*display_viewmodel_t)(void);
typedef void (*display_renderer_t)(void);

typedef uint8_t (*menu_visible_t)(void);
typedef void (*menu_action_t)(void*);
typedef void (*menu_event_t)(void);

typedef struct {
  char* name;
  void* icon;
  char suffix;
  menu_visible_t visible_cb;
  menu_action_t action_cb;
  void* state;
} UiMenuItem;

typedef struct {
  uint8_t count;
  const UiMenuItem *menus;
} UiStandardMenu;

typedef const UiMenuItem* (*menu_get_t)(int16_t);
typedef int16_t (*menu_count_t)(void);
typedef const UiMenuItem* (*menu_get_next_t)(void);
typedef void (*menu_close_t)(void);

typedef struct {
  display_viewmodel_t   viewmodel;
  display_renderer_t  renderer;
  UiParts changed_parts;
  char filename[20];
  union
  {
    struct {
      systime_t subscreen_time;
      struct {
        uint8_t version;
        char text[64];
        bool_t estopped;
        struct {
          systime_t time;
          int16_t offset;
          int16_t length;
          uint8_t delay;
        } marquee;
      } status;
      PrintingSource source;
      struct {
        uint32_t total;
        uint32_t processed;
      } progress;
      char* status_icon;
      int16_t time_spent;
      DashboardAxis axes[3];
      RadStorageHost storage_state;
      DashboardTempData temps[RAD_NUMBER_TEMPERATURES];
      uint8_t active_extruder;
      uint8_t temps_screen;
    } dashboard;
    struct {
      union {
        struct {
          const UiStandardMenu* menu;
          int8_t last_index;
        } standard;
        struct {
          UiMenuItem item;
          int16_t directory_opened;
        } print;
      };
      menu_get_t get_cb;
      menu_count_t count_cb;
      menu_get_next_t get_next_cb;
      menu_close_t close_cb;
      menu_event_t back_cb;
      systime_t last_refresh;
      int8_t pos;
      int16_t encoder_delta;
      int16_t offset;
      int16_t current;
    } menu;
  };
} UiState;

static UiState uiState;
#endif

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/

#if HAS_DISPLAY

static void uiChangePage(display_viewmodel_t viewmodel)
{
  uiState.viewmodel = viewmodel;
  uiState.renderer = NULL;
  uiState.changed_parts = UI_PARTS_All;
}

#include "ui/menu.h"
#include "ui/display_format.h"
#include "display.h"

static msg_t threadDisplay(void *arg) {
  (void)arg;
  bool_t old_estop_state = FALSE;
  systime_t next;
  systime_t now;

  chRegSetThreadName("ui");
  display_ui_init();

  while (TRUE) {
    next = chTimeNow() + MS2ST(100); // 10 fps
    do
    {
      if (!old_estop_state && (old_estop_state = printerIsEstopped()) != FALSE)
      {
        uiChangePage(ui_dashboard_viewmodel);
      }
      uiState.viewmodel();
    } while (uiState.renderer == NULL);
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

#if HAS_DISPLAY
static void uiInputInit(void)
{
  ENABLE_INPUT(generic_wheel);
  ENABLE_INPUT(up_button);
  ENABLE_INPUT(down_button);
  ENABLE_INPUT(left_button);
  ENABLE_INPUT(right_button);
  ENABLE_INPUT(back_button);
  ENABLE_INPUT(enter_button);
}
#endif

/*===========================================================================*/
/* Exported functions.                                                       */
/*===========================================================================*/

void uiInit(void)
{
#if HAS_DISPLAY
  gfxInit();
  uiInputInit();
  uiSetContrast(machine.ui.contrast);

  uiChangePage(ui_dashboard_viewmodel);
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

void uiStateSetActiveFilename(const char* filename)
{
#if !HAS_DISPLAY
  (void) filename;
#else
  // this variable is read only after printer source change event.
  // should have no race condition issue
  uiState.filename[19] = 0;
  strncpy(uiState.filename, filename, 19);
#endif
}
/** @} */
