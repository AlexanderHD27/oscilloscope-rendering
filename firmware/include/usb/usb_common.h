#include "pico/types.h"

#ifndef INCLUDE_USB_COMMON
#define INCLUDE_USB_COMMON

/**
 * @file usb_common.h
 * @author AlexanderHD27
 * @brief Contains USB common types e.g. Package formats
 * @version 0.1
 * @date 2024-02-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

// USB Direction Prefixes
#define USB_DIR_IN 0x80  // Device -> Host
#define USB_DIR_OUT 0x00 // Host -> Device

// Transfer Types
#define USB_TRANSFER_TYPE_CONTROL 0x0
#define USB_TRANSFER_TYPE_ISOCHRONOUS 0x1
#define USB_TRANSFER_TYPE_BULK 0x2
#define USB_TRANSFER_TYPE_INTERRUPT 0x3
#define USB_TRANSFER_TYPE_BITS 0x3


enum SetupPackageType {
    Standard = 0x00,
    Class    = 0x20,
    Vendor   = 0x40,
    Reserved = 0x60
};

enum SetupPackageRequestTypes {
    GET_CONFIGURATION = 8,
    GET_DESCRIPTOR = 6,
    GET_INTERFACE = 10,
    GET_STATUS = 0,

    SET_ADDRESS = 5,
    SET_CONFIGURATION = 9,
    SET_DESCRIPTOR = 7,
    SET_FEATURE = 3,
    SET_INTERFACE = 11,

    CLEAR_FEATURE = 1,
    SYNCH_FRAME = 12
};

struct usb_setupPacket {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __packed;

typedef struct usb_setupPacket usb_setupPacket_t;


// Descriptor types
#define USB_DESCRIPTOR_TYPE_DEVICE 0x01
#define USB_DESCRIPTOR_TYPE_CONFIG 0x02
#define USB_DESCRIPTOR_TYPE_STRING 0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT 0x05

struct usb_deviceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __packed;

struct usb_configurationDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __packed;

struct usb_interfaceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __packed;

struct usb_endpointDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __packed;

struct usb_endpointConfiguration {
    struct usb_endpointDescriptor * descriptor;
    volatile uint32_t * endpointCTRL;
    volatile uint32_t * bufferCTRL;
    volatile uint8_t * dataBuffer;
    uint8_t nextPid;
};

#define NUM_ENDPOINTS 2

struct usb_deviceData {
    struct usb_deviceDescriptor * deviceDesc;
    struct usb_configurationDescriptor * configDesc;
    struct usb_interfaceDescriptor * interfaceDesc;
    uint8_t langDesc;

    struct usb_endpointConfiguration endpoints[NUM_ENDPOINTS];

    uint8_t ** strings;
};

// Configurations

#endif
