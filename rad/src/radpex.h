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
 * @file    src/radpex.h
 * @brief   RAD Ports Abstraction Layer Extended macro.
 * @note    Providers interrupt safe version of unsafe PAL macros
 *
 * @addtogroup RAD_PEX
 * @{
 */

#ifndef _RADPAL_H_
#define _RADPAL_H_

#include "hal.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief Define a IO pin
 */
typedef struct {
  /**
   * @brief   The PIO port
   */
  ioportid_t  port;

  /**
   * @brief   The pin number
   */
  uint8_t     pin;
} pin_t;

typedef struct {
  pin_t       pin;
  uint8_t     active_low;
} signal_t;

/*===========================================================================*/
/* Macro declarations.                                                       */
/*===========================================================================*/

#if PAL_PAD_IS_ATOMIC
#define pexSysLock() do {} while (0)
#define pexSysUnlock() do {} while (0)
#define pexSysLockFromIsr() do {} while (0)
#define pexSysUnlockFromIsr() do {} while (0)
#else
#define pexSysLock() do { chSysLock(); } while (0)
#define pexSysUnlock() do { chSysUnlock(); } while (0)
#define pexSysLockFromIsr() do { chSysLockFromIsr(); } while (0)
#define pexSysUnlockFromIsr() do { chSysUnlockFromIsr(); } while (0)
#endif

#define pexSetPort(port, bits) \
  do { pexSysLock(); palSetPort(port, bits); pexSysUnlock(); } while (0)
#define pexClearPort(port, bits) \
  do { pexSysLock(); palClearPort(port, bits); pexSysUnlock(); } while (0)
#define pexTogglePort(port, bits) \
  do { pexSysLock(); palToggleort(port, bits); pexSysUnlock(); } while (0)
#define pexWritePad(port, pad, bit) \
  do { pexSysLock(); palWritePad(port, pad, bit); pexSysUnlock(); } while (0)
#define pexSetPad(port, pad) \
  do { pexSysLock(); palSetPad(port, pad); pexSysUnlock(); } while (0)
#define pexClearPad(port, pad) \
  do { pexSysLock(); palClearPad(port, pad); pexSysUnlock(); } while (0)
#define pexTogglePad(port, pad) \
  do { pexSysLock(); palTogglePad(port, pad); pexSysUnlock(); } while (0)

#define pexSetPortI(port, bits) \
  do { pexSysLockFromIsr(); palSetPort(port, bits); pexSysUnlockFromIsr(); } while (0)
#define pexClearPortI(port, bits) \
  do { pexSysLockFromIsr(); palClearPort(port, bits); pexSysUnlockFromIsr(); } while (0)
#define pexToggleortI(port, bits) \
  do { pexSysLockFromIsr(); palTogglePort(port, bits); pexSysUnlockFromIsr(); } while (0)
#define pexWritePadI(port, pad, bit) \
  do { pexSysLockFromIsr(); palWritePad(port, pad, bit); pexSysUnlockFromIsr(); } while (0)
#define pexSetPadI(port, pad) \
  do { pexSysLockFromIsr(); palSetPad(port, pad); pexSysUnlockFromIsr(); } while (0)
#define pexClearPadI(port, pad) \
  do { pexSysLockFromIsr(); palClearPad(port, pad); pexSysUnlockFromIsr(); } while (0)
#define pexTogglePadI(port, pad) \
  do { pexSysLockFromIsr(); palTogglePad(port, pad); pexSysUnlockFromIsr(); } while (0)

#define palHasPin(p) ((p).port != 0)

#define palHasSig(sig) ((sig).pin.port != 0)

#define palEnableSig(sig) \
  do { \
    if ((sig).active_low) palClearPad((sig).pin.port, (sig).pin.pin); \
    else palSetPad((sig).pin.port, (sig).pin.pin); \
  } while (0)

#define palDisableSig(sig) \
  do { \
    if ((sig).active_low) palSetPad((sig).pin.port, (sig).pin.pin); \
    else palClearPad((sig).pin.port, (sig).pin.pin); \
  } while (0)

#define pexEnableSig(sig) \
  do { pexSysLock(); palEnableSig(sig); pexSysUnlock(); } while (0)
#define pexDisableSig(sig) \
  do { pexSysLock(); palDisableSig(sig); pexSysUnlock(); } while (0)

#define palSetPinMode(p, mode) \
  palSetPadMode((p).port, (p).pin, mode)

#define palSetSigMode(sig, mode) \
  palSetPinMode((sig).pin, mode)

#define palReadPin(p, active_low) \
  (palReadPad((p).port, (p).pin) ? (!(active_low)) : (active_low))
#define palReadSig(sig) \
  (palReadPin((sig).pin, (sig).active_low))

#endif  /* _RADPEX_H_ */
/** @} */
