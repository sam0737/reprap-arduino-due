/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _USBCFG_H_
#define _USBCFG_H_

extern const USBConfig usbcfg;
extern const SerialUSBConfig serusb_datacfg;
extern const SerialUSBConfig serusb_shellcfg;

// USB Device
extern SerialUSBDriver SDU_SHELL;
extern SerialUSBDriver SDU_DATA;

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
/*
    // Mass Storage Interface
    USB_Descriptor_Interface_t               MS_Interface;
    USB_Descriptor_Endpoint_t                MS_DataInEndpoint;
    USB_Descriptor_Endpoint_t                MS_DataOutEndpoint;*/
} USB_Descriptor_Configuration_t;

#endif  /* _USBCFG_H_ */

/** @} */
