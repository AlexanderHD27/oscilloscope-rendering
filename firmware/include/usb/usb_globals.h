#include <stdio.h>
#include "pico/types.h"

#include "usb.h"

extern uint8_t deviceAddress;
extern bool addressSet;
extern bool configured;

extern struct usb_endpointDescriptor ep0InDescriptor;
extern struct usb_endpointDescriptor ep0OutDescriptor;
extern struct usb_interfaceDescriptor interfaceDesc;
extern struct usb_configurationDescriptor configDesc;
extern struct usb_deviceDescriptor deviceDesc;
extern struct usb_deviceData deviceData;
