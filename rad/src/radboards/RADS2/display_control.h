/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _DISPLAY_CONTROL_H
#define _DISPLAY_CONTROL_H

#include "ch.h"
#include "hal.h"

static void display_control_init(void)
{
}

/*
static void display_lld_set_power(bool_t power)
{
  (void) power;
}
*/

static void display_lld_set_contrast(float contrast)
{
  (void) contrast;
  // gdispControl(GDISP_CONTROL_CONTRAST, (void*)(size_t)(contrast * 100));
}

/*
static void display_lld_set_brightness(float brightness)
{
  (void) brightness;
}
*/

#endif /* _DISPLAY_CONTROL_H */
