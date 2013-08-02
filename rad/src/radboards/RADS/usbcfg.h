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
 * @file    RADS/usbcfg.c
 * @brief   USB Configuration
 *
 * @addtogroup USB_CFG
 * @{
 */

#ifndef _USBCFG_H_
#define _USBCFG_H_

extern const USBConfig usbcfg;
extern SerialUSBConfig serusb_datacfg;
extern SerialUSBConfig serusb_shellcfg;
extern USBMassStorageConfig ums_cfg;

// USB Device
extern SerialUSBDriver SDU_SHELL;
extern SerialUSBDriver SDU_DATA;
extern USBMassStorageDriver UMSD;

#include "LUFA/drivers/USB/USB.h"
/* Macros: */
/** Size in bytes of the CDC device-to-host notification IN endpoint. */
#define CDC_NOTIFICATION_EPSIZE        16

/** Size in bytes of the CDC data IN and OUT endpoints. */
#define CDC_TXRX_EPSIZE                512

/** Size in bytes of the Mass Storage data endpoints. */
#define MASS_STORAGE_IO_EPSIZE         512

/* Type Defines: */
/** Type define for the device configuration descriptor structure. This must be defined in the
 *  application code, as the configuration descriptor contains several sub-descriptors which
 *  vary between devices, and which describe the device's usage to the host.
 */
typedef struct
{
    USB_Descriptor_Configuration_Header_t    Config;

    // First CDC Control Interface
    USB_Descriptor_Interface_Association_t   CDC1_IAD;
    USB_Descriptor_Interface_t               CDC1_CCI_Interface;
    USB_CDC_Descriptor_FunctionalHeader_t    CDC1_Functional_Header;
    USB_CDC_Descriptor_FunctionalACM_t       CDC1_Functional_ACM;
    USB_CDC_Descriptor_FunctionalUnion_t     CDC1_Functional_Union;
    USB_Descriptor_Endpoint_t                CDC1_ManagementEndpoint;

    // First CDC Data Interface
    USB_Descriptor_Interface_t               CDC1_DCI_Interface;
    USB_Descriptor_Endpoint_t                CDC1_DataOutEndpoint;
    USB_Descriptor_Endpoint_t                CDC1_DataInEndpoint;

    // Second CDC Control Interface
    USB_Descriptor_Interface_Association_t   CDC2_IAD;
    USB_Descriptor_Interface_t               CDC2_CCI_Interface;
    USB_CDC_Descriptor_FunctionalHeader_t    CDC2_Functional_Header;
    USB_CDC_Descriptor_FunctionalACM_t       CDC2_Functional_ACM;
    USB_CDC_Descriptor_FunctionalUnion_t     CDC2_Functional_Union;
    USB_Descriptor_Endpoint_t                CDC2_ManagementEndpoint;

    // Second CDC Data Interface
    USB_Descriptor_Interface_t               CDC2_DCI_Interface;
    USB_Descriptor_Endpoint_t                CDC2_DataOutEndpoint;
    USB_Descriptor_Endpoint_t                CDC2_DataInEndpoint;

    // Mass Storage Interface
    USB_Descriptor_Interface_t               MS_Interface;
    USB_Descriptor_Endpoint_t                MS_DataInEndpoint;
    USB_Descriptor_Endpoint_t                MS_DataOutEndpoint;
} USB_Descriptor_Configuration_t;

#endif  /* _USBCFG_H_ */

/** @} */
