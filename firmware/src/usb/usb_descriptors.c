#include "pico/types.h"
#include "pico/unique_id.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb.h"

// at least 2.1 or 3.x for BOS & webUSB
#define USB_BCD 0x0200

// --------------------------------------------------------------------+
// Device Descriptor
// --------------------------------------------------------------------+

const tusb_desc_device_t descriptorDeviceFullspeed = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = 0xBabe,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// --------------------------------------------------------------------+
// Config Descriptor
// --------------------------------------------------------------------+

// Endpoint Numbers
#define EP_NUM_CDC_CONFIG_NOTIF   0x81
#define EP_NUM_CDC_CONFIG_OUT     0x02
#define EP_NUM_CDC_CONFIG_IN      0x82

enum {
    ITF_NUM_CDC_0 = 0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_TOTAL
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

uint8_t const descriptorConfigurationFullspeed[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, CONFIG0_NAME, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, INTERFACE0_NAME, EP_NUM_CDC_CONFIG_NOTIF, 8, EP_NUM_CDC_CONFIG_OUT, EP_NUM_CDC_CONFIG_IN, CFG_TUD_CDC_RX_BUFSIZE)
};


// --------------------------------------------------------------------+
// Descriptor Strings
// --------------------------------------------------------------------+

char serialNumberBuffer[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
char const* stringDescriptorArray [] = {
    (const char[]) { 0x09, 0x04 },
    "TinyUSB",
    "OSCI-Vector-Display",
    serialNumberBuffer,
    "Instruction Configuration",
    "Vector Data",
};





