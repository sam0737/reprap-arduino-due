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

#include "ch.h"
#include "hal.h"
#include "usb_cdc.h"
#include "usbcfg.h"

/*
 * USB Device Descriptor.
 */
/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
static const USB_Descriptor_Device_t DeviceDescriptor =
{
  .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

  .USBSpecification       = VERSION_BCD(02.00),
  .Class                  = CDC_CSCP_CDCClass,
  .SubClass               = CDC_CSCP_NoSpecificSubclass,
  .Protocol               = CDC_CSCP_NoSpecificProtocol,

  .Endpoint0Size          = 0x40,

  .VendorID               = 0x2341,
  .ProductID              = 0x003b,
  .ReleaseNumber          = VERSION_BCD(00.02),

  .ManufacturerStrIndex   = 0x01,
  .ProductStrIndex        = 0x02,
  .SerialNumStrIndex      = 0x03,

  .NumberOfConfigurations = 1
};


/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t ConfigurationDescriptor =
{
  .Config =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

      .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
      .TotalInterfaces        = 2,

      .ConfigurationNumber    = 1,
      .ConfigurationStrIndex  = NO_DESCRIPTOR,

      .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

      .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
    },

  .CDC_CCI_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

      .InterfaceNumber        = 0,
      .AlternateSetting       = 0,

      .TotalEndpoints         = 1,

      .Class                  = CDC_CSCP_CDCClass,
      .SubClass               = CDC_CSCP_ACMSubclass,
      .Protocol               = CDC_CSCP_ATCommandProtocol,

      .InterfaceStrIndex      = NO_DESCRIPTOR
    },

  .CDC_Functional_Header =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_Header,

      .CDCSpecification       = VERSION_BCD(01.10),
    },

  .CDC_Functional_ACM =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_ACM,

      .Capabilities           = 0x06,
    },

  .CDC_Functional_Union =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_Union,

      .MasterInterfaceNumber  = 0,
      .SlaveInterfaceNumber   = 1,
    },

  .CDC_NotificationEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

      .EndpointAddress        = CDC_NOTIFICATION_EPADDR,
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_NOTIFICATION_EPSIZE,
      .PollingIntervalMS      = 0x10
    },

  .CDC_DCI_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

      .InterfaceNumber        = 1,
      .AlternateSetting       = 0,

      .TotalEndpoints         = 2,

      .Class                  = CDC_CSCP_CDCDataClass,
      .SubClass               = CDC_CSCP_NoDataSubclass,
      .Protocol               = CDC_CSCP_NoDataProtocol,

      .InterfaceStrIndex      = NO_DESCRIPTOR
    },

  .CDC_DataOutEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

      .EndpointAddress        = CDC_RX_EPADDR,
      .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_TXRX_EPSIZE,
      .PollingIntervalMS      = 0x05
    },

  .CDC_DataInEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

      .EndpointAddress        = CDC_TX_EPADDR,
      .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_TXRX_EPSIZE,
      .PollingIntervalMS      = 0x05
    }
};

// U.S. English language identifier.
static const USB_Descriptor_String_t LanguageString  =
{
  .Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},
  .UnicodeString          = "\x09\x04"
};

// Vendor
static const USB_Descriptor_String_t VendorString =
{
  .Header                 = {.Size = USB_STRING_LEN(3), .Type = DTYPE_String},
  .UnicodeString          = "R\0A\0D\0"
};

// Product
static const USB_Descriptor_String_t ProductString =
{
  .Header                 = {.Size = USB_STRING_LEN(13), .Type = DTYPE_String},
  .UnicodeString          = "L\0U\0F\0A\0 \0C\0D\0C\0 \0D\0e\0m\0o\0"
};

// Version
static const USB_Descriptor_String_t VersionString =
{
  .Header                 = {.Size = USB_STRING_LEN(5), .Type = DTYPE_String},
  .UnicodeString          = "0\x00.\x000\x00.\x001\x00"
};

/*
 * Strings wrappers array.
 */
static const USB_Descriptor_String_t* vcom_strings[] =
{
  &LanguageString,
  &VendorString,
  &ProductString,
  &VendorString
};

static USBDescriptor descriptor_stash;
/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {
  (void)usbp;
  (void)lang;
  size_t size = 0;
  void* ptr = NULL;

  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    ptr = (void*)&DeviceDescriptor;
    size = DeviceDescriptor.Header.Size;
    break;
  case USB_DESCRIPTOR_CONFIGURATION:
    ptr = (void*)&ConfigurationDescriptor;
    size = ConfigurationDescriptor.Config.TotalConfigurationSize;
    break;
  case USB_DESCRIPTOR_STRING:
    if (dindex < 4) {
      ptr = (void*)vcom_strings[dindex];
      size = ((USB_Descriptor_String_t*)ptr)->Header.Size;
    }
    break;
  }
  if (ptr == NULL)
    return NULL;
  descriptor_stash.ud_size = size;
  descriptor_stash.ud_string = ptr;
  return &descriptor_stash;
}

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep_out_state;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep_out_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  NULL,
  sduDataReceived,
  0,
  CDC_TXRX_EPSIZE,
  NULL,
  &ep_out_state,
  // Optional
  1
};

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep_in_state;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep_in_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  NULL,
  CDC_TXRX_EPSIZE,
  0x0000,
  &ep_in_state,
  NULL,
  // Optional
  1
};

/**
 * @brief   IN EP2 state.
 */
static USBInEndpointState ep_int_instate;

/**
 * @brief   EP2 initialization structure (IN only).
 */
static const USBEndpointConfig ep_int_config = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  sduInterruptTransmitted,
  NULL,
  CDC_NOTIFICATION_EPSIZE,
  0x0000,
  &ep_int_instate,
  NULL,
  // Optional
  0
};

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
  uint8_t ep;

  switch (event) {
  case USB_EVENT_RESET:
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromIsr();

    for (ep = 1; ep < 10; ep++)
    {
      /* Enables the endpoints specified into the configuration.
         Note, this callback is invoked from an ISR so I-Class functions
         must be used.*/
      if (ep == USB_CDC_DATA_AVAILABLE_EP)
        usbInitEndpointI(usbp, USB_CDC_DATA_AVAILABLE_EP, &ep_out_config);
      if (ep == USB_CDC_DATA_REQUEST_EP)
        usbInitEndpointI(usbp, USB_CDC_DATA_REQUEST_EP, &ep_in_config);
      if (ep == USB_CDC_INTERRUPT_REQUEST_EP)
        usbInitEndpointI(usbp, USB_CDC_INTERRUPT_REQUEST_EP, &ep_int_config);
    }

    /* Resetting the state of the CDC subsystem.*/
    sduConfigureHookI(usbp);
    chSysUnlockFromIsr();
    return;
  case USB_EVENT_SUSPEND:
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  sduRequestsHook,
  NULL,
  .irq_priority = 0
};

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig serusbcfg = {
  &USBD1
};
