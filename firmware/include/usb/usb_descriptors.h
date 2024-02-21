#include "pico/types.h"
#include "hardware/structs/usb.h"

#include "usb.h"
#include "usb_common.h"

#ifndef USB_INCLUDE_DESCRIPTOR
#define USB_INCLUDE_DESCRIPTOR

// Endpoint 0 Device -> Host
#define EP0_IN_ADDR  (0x00 | USB_DIR_IN )

// Endpoint 1 Host -> Device
#define EP0_OUT_ADDR (0x00 | USB_DIR_OUT)

enum usb_stringsIndex {
    LANG_DESCRIPTOR = 0,
    PRODUCT_NAME,
    MANUFACTURER_NAME,
    CONFIG1_NAME,
};


#endif