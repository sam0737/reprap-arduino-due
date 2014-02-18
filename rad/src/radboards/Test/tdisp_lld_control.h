/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _TDISP_LLD_CONTROL_H
#define _TDISP_LLD_CONTROL_H

#include "ch.h"
#include "hal.h"
#include "httpmmap.h"

void* display_state;
size_t display_state_size;

HttpMmapObject hmo = { .name = "display" };

static void display_control_init(void)
{
  if (display_state != NULL && display_state_size != 0)
  {
    hmo.buffer = display_state;
    hmo.size = display_state_size;
    httpmmapAdd(&hmd, &hmo);
  }
}

/*
static void display_lld_set_power(bool_t power)
{
  (void) power;
}
*/

static void display_lld_set_contrast(float contrast)
{
}

/*
static void display_lld_set_brightness(float brightness)
{
  (void) brightness;
}
*/

#endif /* _TDISP_LLD_CONTROL_H */
