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
extern const SerialUSBConfig serusbcfg;

#include "LUFA/drivers/USB/USB.h"

/* Macros: */
  /** Endpoint address of the CDC device-to-host notification IN endpoint. */
  #define CDC_NOTIFICATION_EPADDR        (ENDPOINT_DIR_IN  | USB_CDC_INTERRUPT_REQUEST_EP)

  /** Endpoint address of the CDC device-to-host data IN endpoint. */
  #define CDC_TX_EPADDR                  (ENDPOINT_DIR_IN  | USB_CDC_DATA_REQUEST_EP)

  /** Endpoint address of the CDC host-to-device data OUT endpoint. */
  #define CDC_RX_EPADDR                  (ENDPOINT_DIR_OUT | USB_CDC_DATA_AVAILABLE_EP)

  /** Size in bytes of the CDC device-to-host notification IN endpoint. */
  #define CDC_NOTIFICATION_EPSIZE        16

  /** Size in bytes of the CDC data IN and OUT endpoints. */
  #define CDC_TXRX_EPSIZE                512

/* Type Defines: */
  /** Type define for the device configuration descriptor structure. This must be defined in the
   *  application code, as the configuration descriptor contains several sub-descriptors which
   *  vary between devices, and which describe the device's usage to the host.
   */
  typedef struct
  {
    USB_Descriptor_Configuration_Header_t    Config;

    // CDC Control Interface
    USB_Descriptor_Interface_t               CDC_CCI_Interface;
    USB_CDC_Descriptor_FunctionalHeader_t    CDC_Functional_Header;
    USB_CDC_Descriptor_FunctionalACM_t       CDC_Functional_ACM;
    USB_CDC_Descriptor_FunctionalUnion_t     CDC_Functional_Union;
    USB_Descriptor_Endpoint_t                CDC_NotificationEndpoint;

    // CDC Data Interface
    USB_Descriptor_Interface_t               CDC_DCI_Interface;
    USB_Descriptor_Endpoint_t                CDC_DataOutEndpoint;
    USB_Descriptor_Endpoint_t                CDC_DataInEndpoint;
  } USB_Descriptor_Configuration_t;

#endif  /* _USBCFG_H_ */

/** @} */
