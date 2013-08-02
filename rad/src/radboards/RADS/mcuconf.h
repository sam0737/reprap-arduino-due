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
#define SAM3XA_PWM_USE_CH5        TRUE
#define SAM3XA_PWM_USE_CH6        TRUE

#define SAM3XA_SPI_USE_SPI1       TRUE

#define SAM3XA_SERIAL_USE_UART    TRUE
#define SAM3XA_USB_USE_UOTGHS     TRUE

/**
 * USB
 */

// Endpoints Address
/** Endpoint address of the CDC 1 device-to-host notification IN endpoint. */
#define CDC1_NOTIFICATION_EPADDR       8
/** Endpoint address of the CDC 1 device-to-host data IN endpoint. */
#define CDC1_TX_EPADDR                 3
/** Endpoint address of the CDC 1 host-to-device data OUT endpoint. */
#define CDC1_RX_EPADDR                 4

/** Endpoint address of the CDC 2 device-to-host notification IN endpoint. */
#define CDC2_NOTIFICATION_EPADDR       9
/** Endpoint address of the CDC 2 device-to-host data IN endpoint. */
#define CDC2_TX_EPADDR                 5
/** Endpoint address of the CDC 2 host-to-device data OUT endpoint. */
#define CDC2_RX_EPADDR                 6

/** Endpoint address of the Mass Storage device-to-host data IN endpoint. */
#define MASS_STORAGE_IN_EPADDR         1
/** Endpoint address of the Mass Storage host-to-device data OUT endpoint. */
#define MASS_STORAGE_OUT_EPADDR        2

#endif  /* _MCUCONF_H_ */

/** @} */
