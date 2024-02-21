#include "pico/types.h"
#include "usb_descriptors.h"

uint8_t * strings[] = {
    (uint8_t *) "\x09\x04",
    (uint8_t *) "Scope-Display",
    (uint8_t *) "AlexanderHD27",
    (uint8_t *) "Instruction Mode"
};

// EP0 IN
struct usb_endpointDescriptor ep0InDescriptor = {
    .bLength          = sizeof(struct usb_endpointDescriptor),
    .bDescriptorType  = USB_DESCRIPTOR_TYPE_ENDPOINT,
    .bEndpointAddress = EP0_IN_ADDR,
    .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
    .wMaxPacketSize   = 64,
    .bInterval        = 0
};

// EP0 OUT
struct usb_endpointDescriptor ep0OutDescriptor = {
    .bLength          = sizeof(struct usb_endpointDescriptor),
    .bDescriptorType  = USB_DESCRIPTOR_TYPE_ENDPOINT,
    .bEndpointAddress = EP0_OUT_ADDR,
    .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
    .wMaxPacketSize   = 64,
    .bInterval        = 0
};


// Interface Descriptor
struct usb_interfaceDescriptor interfaceDesc = {
    .bLength            = sizeof(struct usb_interfaceDescriptor),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_INTERFACE,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = 0xff, // Vendor specific endpoint
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface         = 0
};

// Config Descriptor
struct usb_configurationDescriptor configDesc = {
    .bLength         = sizeof(struct usb_configurationDescriptor),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIG,
    .wTotalLength    = (sizeof(configDesc) +
                        sizeof(interfaceDesc)),
    .bNumInterfaces  = 1,
    .bConfigurationValue = 1, // Configuration 1
    .iConfiguration = CONFIG1_NAME,      // No string
    .bmAttributes = 0xc0,     // attributes: self powered, no remote wakeup
    .bMaxPower = 0x32         // 100ma
};

// Device Descriptor
struct usb_deviceDescriptor deviceDesc = { 
    .bLength            = sizeof(struct usb_deviceDescriptor),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_DEVICE,
    .bcdUSB             = 0x0110,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0xcafe,
    .idProduct          = 0xbabe,
    .bcdDevice          = 0x0100,
    .iManufacturer      = MANUFACTURER_NAME,
    .iProduct           = PRODUCT_NAME,
    .iSerialNumber      = 0x42,
    .bNumConfigurations = 1
};


// Device Configuration

struct usb_deviceData deviceData = {
    .deviceDesc = &deviceDesc,
    .configDesc = &configDesc,
    .interfaceDesc = &interfaceDesc,

    .langDesc = LANG_DESCRIPTOR,
    .strings = strings,

    // Endpoint Configuration
    .endpoints = {
        { // Endpoint 0 IN
            .descriptor = &ep0InDescriptor,
            .endpointCTRL = NULL, // NA for EP0
            .bufferCTRL = &usb_dpram->ep_buf_ctrl[0].in,
            // EP0 in and out share a data buffer
            .dataBuffer = &usb_dpram->ep0_buf_a[0],
            .nextPid = 0
        },
        { // Endpoint 0 OUT
            .descriptor = &ep0OutDescriptor,
            .endpointCTRL = NULL, // NA for EP0,
            .bufferCTRL = &usb_dpram->ep_buf_ctrl[0].out,
            // EP0 in and out share a data buffer
            .dataBuffer = &usb_dpram->ep0_buf_a[0],
            .nextPid = 0
        },
    }
};
