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

#include "ch.h"
#include "hal.h"
#include "usb_cdc.h"
#if HAL_USE_MSD
#include "usb_msd.h"
#endif
#include "usbcfg.h"

#include "mcuconf.h"

static bool_t usbRequestsHook(USBDriver *usbp);

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
  .Class                  = USB_CSCP_IADDeviceClass,
  .SubClass               = USB_CSCP_IADDeviceSubclass,
  .Protocol               = USB_CSCP_IADDeviceProtocol,

  .Endpoint0Size          = 0x40,

  .VendorID               = 0x03EB,
  .ProductID              = 0x2040,
  .ReleaseNumber          = VERSION_BCD(68.01),

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
        .TotalInterfaces        =
            4
#if HAL_USE_MSD
            + 1
#endif
            ,

        .ConfigurationNumber    = 1,
        .ConfigurationStrIndex  = NO_DESCRIPTOR,

        .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

        .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
    },

    .CDC1_IAD =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_Association_t), .Type = DTYPE_InterfaceAssociation},

        .FirstInterfaceIndex    = 0,
        .TotalInterfaces        = 2,

        .Class                  = CDC_CSCP_CDCClass,
        .SubClass               = CDC_CSCP_ACMSubclass,
        .Protocol               = CDC_CSCP_ATCommandProtocol,

        .IADStrIndex            = NO_DESCRIPTOR
    },

    .CDC1_CCI_Interface =
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

    .CDC1_Functional_Header =
    {
        .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
        .Subtype                = CDC_DSUBTYPE_CSInterface_Header,

        .CDCSpecification       = VERSION_BCD(01.10),
    },

    .CDC1_Functional_ACM =
    {
        .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
        .Subtype                = CDC_DSUBTYPE_CSInterface_ACM,

        .Capabilities           = 0x06,
    },

    .CDC1_Functional_Union =
    {
        .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
        .Subtype                = CDC_DSUBTYPE_CSInterface_Union,

        .MasterInterfaceNumber  = 0,
        .SlaveInterfaceNumber   = 1,
    },

    .CDC1_ManagementEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_IN | CDC1_NOTIFICATION_EPADDR,
        .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = CDC_NOTIFICATION_EPSIZE,
        .PollingIntervalMS      = 0xFF
    },

    .CDC1_DCI_Interface =
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

    .CDC1_DataOutEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_OUT | CDC1_RX_EPADDR,
        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = CDC_TXRX_EPSIZE,
        .PollingIntervalMS      = 0x05
    },

    .CDC1_DataInEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_IN | CDC1_TX_EPADDR,
        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = CDC_TXRX_EPSIZE,
        .PollingIntervalMS      = 0x05
    },

    .CDC2_IAD =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_Association_t), .Type = DTYPE_InterfaceAssociation},

        .FirstInterfaceIndex    = 2,
        .TotalInterfaces        = 2,

        .Class                  = CDC_CSCP_CDCClass,
        .SubClass               = CDC_CSCP_ACMSubclass,
        .Protocol               = CDC_CSCP_ATCommandProtocol,

        .IADStrIndex            = NO_DESCRIPTOR
    },

    .CDC2_CCI_Interface =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

        .InterfaceNumber        = 2,
        .AlternateSetting       = 0,

        .TotalEndpoints         = 1,

        .Class                  = CDC_CSCP_CDCClass,
        .SubClass               = CDC_CSCP_ACMSubclass,
        .Protocol               = CDC_CSCP_ATCommandProtocol,

        .InterfaceStrIndex      = NO_DESCRIPTOR
    },

    .CDC2_Functional_Header =
    {
        .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
        .Subtype                = CDC_DSUBTYPE_CSInterface_Header,

        .CDCSpecification       = VERSION_BCD(01.10),
    },

    .CDC2_Functional_ACM =
    {
        .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
        .Subtype                = CDC_DSUBTYPE_CSInterface_ACM,

        .Capabilities           = 0x06,
    },

    .CDC2_Functional_Union =
    {
        .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
        .Subtype                = CDC_DSUBTYPE_CSInterface_Union,

        .MasterInterfaceNumber  = 2,
        .SlaveInterfaceNumber   = 3,
    },

    .CDC2_ManagementEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_IN | CDC2_NOTIFICATION_EPADDR,
        .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = CDC_NOTIFICATION_EPSIZE,
        .PollingIntervalMS      = 0xFF
    },

    .CDC2_DCI_Interface =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

        .InterfaceNumber        = 3,
        .AlternateSetting       = 0,

        .TotalEndpoints         = 2,

        .Class                  = CDC_CSCP_CDCDataClass,
        .SubClass               = CDC_CSCP_NoDataSubclass,
        .Protocol               = CDC_CSCP_NoDataProtocol,

        .InterfaceStrIndex      = NO_DESCRIPTOR
    },

    .CDC2_DataOutEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_OUT | CDC2_RX_EPADDR,
        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = CDC_TXRX_EPSIZE,
        .PollingIntervalMS      = 0x05
    },

    .CDC2_DataInEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_IN | CDC2_TX_EPADDR,
        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = CDC_TXRX_EPSIZE,
        .PollingIntervalMS      = 0x05
    },

#if HAL_USE_MSD
    .MS_Interface =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

        .InterfaceNumber        = 4,
        .AlternateSetting       = 0,

        .TotalEndpoints         = 2,

        .Class                  = MS_CSCP_MassStorageClass,
        .SubClass               = MS_CSCP_SCSITransparentSubclass,
        .Protocol               = MS_CSCP_BulkOnlyTransportProtocol,

        .InterfaceStrIndex      = NO_DESCRIPTOR
    },

    .MS_DataInEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_IN | MASS_STORAGE_IN_EPADDR,
        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = MASS_STORAGE_IO_EPSIZE,
        .PollingIntervalMS      = 0x05
    },

    .MS_DataOutEndpoint =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

        .EndpointAddress        = ENDPOINT_DIR_OUT | MASS_STORAGE_OUT_EPADDR,
        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = MASS_STORAGE_IO_EPSIZE,
        .PollingIntervalMS      = 0x05
    }
#endif
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
  .Header                 = {.Size = USB_STRING_LEN(8), .Type = DTYPE_String},
  .UnicodeString          = "R\0A\0D\0 \0D\0e\0m\0o\0"
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
 * Endpoint Configuration
 */

// -- CDC1 --

/**
 * @brief   CDC1 INT state.
 */
static USBInEndpointState cdc1_int_instate;

/**
 * @brief   CDC1 initialization structure (Interrupt).
 */
static const USBEndpointConfig cdc1_ep_int_config = {
  USB_EP_MODE_TYPE_INTR,
  NULL, sduInterruptTransmitted, NULL,
  CDC_NOTIFICATION_EPSIZE, 0,
  &cdc1_int_instate, NULL,
  // Optional
  0
};

/**
 * @brief   CDC1 IN state.
 */
static USBInEndpointState cdc1_in_state;

/**
 * @brief   CDC1 initialization structure (TXIN).
 */
static const USBEndpointConfig cdc1_ep_in_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL, sduDataTransmitted, NULL,
  CDC_TXRX_EPSIZE, 0,
  &cdc1_in_state, NULL,
  // Bank
  1
};

/**
 * @brief   CDC1 OUT state.
 */
static USBOutEndpointState cdc1_out_state;

/**
 * @brief   CDC1 initialization structure (RXOUT)
 */
static const USBEndpointConfig cdc1_ep_out_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL, NULL, sduDataReceived,
  0, CDC_TXRX_EPSIZE,
  NULL, &cdc1_out_state,
  // Bank
  0
};

// -- CDC2 --

/**
 * @brief   CDC2 INT state.
 */
static USBInEndpointState cdc2_int_instate;

/**
 * @brief   CDC2 initialization structure (Interrupt).
 */
static const USBEndpointConfig cdc2_ep_int_config = {
  USB_EP_MODE_TYPE_INTR,
  NULL, sduInterruptTransmitted, NULL,
  CDC_NOTIFICATION_EPSIZE, 0,
  &cdc2_int_instate, NULL,
  // Optional
  0
};

/**
 * @brief   CDC2 IN state.
 */
static USBInEndpointState cdc2_in_state;

/**
 * @brief   CDC2 initialization structure (TXIN).
 */
static const USBEndpointConfig cdc2_ep_in_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL, sduDataTransmitted, NULL,
  CDC_TXRX_EPSIZE, 0,
  &cdc2_in_state, NULL,
  // Bank
  1
};

/**
 * @brief   CDC2 OUT state.
 */
static USBOutEndpointState cdc2_out_state;

/**
 * @brief   CDC2 initialization structure (RXOUT)
 */
static const USBEndpointConfig cdc2_ep_out_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL, NULL, sduDataReceived,
  0, CDC_TXRX_EPSIZE,
  NULL, &cdc2_out_state,
  // Bank
  0
};

#if HAL_USE_MSD
// -- MS --

/**
 * @brief   MS OUT state.
 */
static USBOutEndpointState ms_out_state;

/**
 * @brief   MS initialization structure (OUT)
 */
static const USBEndpointConfig ms_ep_out_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL, NULL, msdUsbEvent,
  0, MASS_STORAGE_IO_EPSIZE,
  NULL, &ms_out_state,
  // Bank
  0
};

/**
 * @brief   MS IN state.
 */
static USBInEndpointState ms_in_state;

/**
 * @brief   MS initialization structure (IN).
 */
static const USBEndpointConfig ms_ep_in_config = {
  USB_EP_MODE_TYPE_BULK,
  NULL, msdUsbEvent, NULL,
  MASS_STORAGE_IO_EPSIZE, 0,
  &ms_in_state, NULL,
  // Bank
  1
};
#endif

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
      // Make sure the endpoint are initialized from 1 to 9
      if (ep == CDC1_NOTIFICATION_EPADDR)
        usbInitEndpointI(usbp, CDC1_NOTIFICATION_EPADDR, &cdc1_ep_int_config);
      if (ep == CDC1_TX_EPADDR)
        usbInitEndpointI(usbp, CDC1_TX_EPADDR, &cdc1_ep_in_config);
      if (ep == CDC1_RX_EPADDR)
        usbInitEndpointI(usbp, CDC1_RX_EPADDR, &cdc1_ep_out_config);

      if (ep == CDC2_NOTIFICATION_EPADDR)
        usbInitEndpointI(usbp, CDC2_NOTIFICATION_EPADDR, &cdc2_ep_int_config);
      if (ep == CDC2_TX_EPADDR)
        usbInitEndpointI(usbp, CDC2_TX_EPADDR, &cdc2_ep_in_config);
      if (ep == CDC2_RX_EPADDR)
        usbInitEndpointI(usbp, CDC2_RX_EPADDR, &cdc2_ep_out_config);

#if HAL_USE_MSD
      if (ep == MASS_STORAGE_IN_EPADDR)
        usbInitEndpointI(usbp, MASS_STORAGE_IN_EPADDR, &ms_ep_in_config);
      if (ep == MASS_STORAGE_OUT_EPADDR)
        usbInitEndpointI(usbp, MASS_STORAGE_OUT_EPADDR, &ms_ep_out_config);
#endif
    }

    /* Resetting the state of the CDC subsystem.*/
    sduConfigureHookI(usbp);
#if HAL_USE_MSD
    msdConfigureHookI(usbp);
#endif
    chSysUnlockFromIsr();
    return;
  case USB_EVENT_SUSPEND:
#if HAL_USE_MSD
    msdSuspendHookI(usbp);
#endif
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
  usbRequestsHook,
  NULL,
  .irq_priority = 0
};

/*
 * Serial over USB driver configuration.
 */
SerialUSBConfig serusb_datacfg = {
    &USBD1, 0,
    CDC1_NOTIFICATION_EPADDR, CDC1_TX_EPADDR, CDC1_RX_EPADDR,
    .linecoding_cb = NULL,
    .controllinestate_cb = NULL
};

SerialUSBConfig serusb_shellcfg = {
    &USBD1, 2,
    CDC2_NOTIFICATION_EPADDR, CDC2_TX_EPADDR, CDC2_RX_EPADDR,
    .linecoding_cb = NULL,
    .controllinestate_cb = NULL
};

#if HAL_USE_MSD
USBMassStorageConfig ums_cfg = {
    &USBD1, 4,
    MASS_STORAGE_OUT_EPADDR, MASS_STORAGE_IN_EPADDR,
    NULL,
    "RADS",
    "RADS",
    "0.1"
};
#endif

static bool_t usbRequestsHook(USBDriver *usbp)
{
  // Composite Interface Number
#if HAL_USE_MSD
  if (usbp->setup[4] == 4)
    return msdRequestsHook(usbp);
#endif
  return sduRequestsHook(usbp);
}

/* Device */
SerialUSBDriver SDU_SHELL;
SerialUSBDriver SDU_DATA;

#if HAL_USE_MSD
USBMassStorageDriver UMSD;
#endif

/** @} */
