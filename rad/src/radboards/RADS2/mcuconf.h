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
 * @file    RADS/mcuconf.h
 * @brief   MCU configuration header.
 *
 * @addtogroup MCU_CONF
 * @{
 */

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

#define SAM3XA_ADC_USE_ADC1       TRUE

#define SAM3XA_PWM_USE_CH0        TRUE
#define SAM3XA_PWM_USE_CH1        TRUE
#define SAM3XA_PWM_USE_CH2        TRUE
#define SAM3XA_PWM_USE_CH3        TRUE
#define SAM3XA_PWM_USE_CH4        TRUE
#define SAM3XA_PWM_USE_CH5        TRUE
#define SAM3XA_PWM_USE_CH6        TRUE
#define SAM3XA_PWM_USE_CH7        TRUE

#define SAM3XA_GPT_USE_TC0        TRUE

#define SAM3XA_SPI_USE_SPI1       TRUE

#define SAM3XA_SERIAL_USE_UART    TRUE
#define SAM3XA_UART_USE_USART0    TRUE
#define SAM3XA_USB_USE_UOTGHS     TRUE

#define SAM3XA_USB_USE_DMAC       TRUE

/**
 * USB
 */

#endif  /* _MCUCONF_H_ */

/** @} */
