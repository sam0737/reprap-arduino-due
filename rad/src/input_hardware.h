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
 * @file    input_hardware.h
 * @brief   Input hardware header
 *
 * @addtogroup INPUT_HARDWARE
 * @{
 */
#ifndef _INPUT_HARDWARE_H_
#define _INPUT_HARDWARE_H_

#include "input.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void inputButtonFetcher(RadInputConfig* config, RadInputState* state);
  void inputEncoderFetcher(RadInputConfig* config, RadInputState* state);
#if GINPUT_NEED_MOUSE
  void inputGinputFetcher(RadInputConfig* config, RadInputState* state);
#elif HAS_HTTPMMAP
  void inputVirtualEncoderFetcher(RadInputConfig* config, RadInputState* state);
#endif
  RadInputValue inputButtonProcessor(RadInputState* state);
  RadInputValue inputEncoderProcessor(RadInputState* state);
#ifdef __cplusplus
}
#endif

#endif  /* _INPUT_HARDWARE_H_ */

/** @} */
