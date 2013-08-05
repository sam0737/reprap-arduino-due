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
 * @file    beeper.h
 * @brief   Beeper header
 *
 * @addtogroup BEEPER
 * @{
 */
#ifndef _RAD_BEEPER_H_
#define _RAD_BEEPER_H_

/*===========================================================================*/
/* Data structures and types.                                                */
/*===========================================================================*/

typedef struct {
  uint16_t tone;
  uint16_t len;
} BeeperNote;

/**
 * @brief   PWM driver channel configuration structure.
 */
typedef struct {
  BeeperNote *notes;
} BeeperTune;

extern const BeeperTune tuneStartup, tuneFinished, tuneOk, tuneWarning;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void beeperInit(void);
  void beeperPlay(const BeeperTune*);
#ifdef __cplusplus
}
#endif

#endif  /* _RAD_BEEPER_H_ */

/** @} */
