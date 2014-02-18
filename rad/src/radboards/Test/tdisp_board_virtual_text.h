/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

extern void* display_state;
extern size_t display_state_size;

static void init_board(void) {
  display_state = &tdisp_state;
  display_state_size = sizeof(tdisp_state);
}
