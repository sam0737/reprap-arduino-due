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

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes RAD, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#define BUS_4BITS   TRUE
#define PORT_RS     IOPORT1
#define PIN_RS      20
#define PORT_EN     IOPORT3
#define PIN_EN      19
#define DATA_SHIFT  15
#define MASK_DATA   (0xF<<DATA_SHIFT)

static void init_board(void) {
  // EN
  palSetPadMode(PORT_EN, PIN_EN, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(PORT_EN, PIN_EN);

  // RS
  palSetPadMode(PORT_RS, PIN_RS, PAL_MODE_OUTPUT_PUSHPULL);

  // D4-D7
  palSetGroupMode(IOPORT3, 0xF, DATA_SHIFT, PAL_MODE_OUTPUT_PUSHPULL);
}

static void _write_to_lcd(uint8_t data) {
  gfxSystemLock();
  palWriteGroup(IOPORT3, 0xF, DATA_SHIFT,
      (data & 0x80 ? 1 : 0) | (data & 0x40 ? 2 : 0) | (data & 0x20 ? 4 : 0) | (data & 0x10 ? 8 : 0)
  );
  gfxSystemUnlock();
  gfxSleepMicroseconds(1);
  palSetPad(PORT_EN, PIN_EN);
  gfxSleepMicroseconds(1);
  palClearPad(PORT_EN, PIN_EN);
}

static void write_cmd(uint8_t data) {
  palClearPad(PORT_RS, PIN_RS);
  _write_to_lcd(data);
  _write_to_lcd(data<<4);
}

static void write_data(uint8_t data) {
  palSetPad(PORT_RS, PIN_RS);
  _write_to_lcd(data);
  _write_to_lcd(data<<4);
}

