#include "pico/types.h"
#include "pico/unique_id.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb.h"

// This is the USB Version used by the devices (v2.0) in Binary Codded Decimal. For BOS & webUSB v2.1 or 3.x are needed
#define USB_BCD 0x0200

// --------------------------------------------------------------------+
// Device Descriptor
// --------------------------------------------------------------------+

/**
 * @brief Defintion for Device Descriptor
 * This is the most top-level descriptor sent to the host as response to a GET DEVICE Request. 
 * VID (Vendor ID) and PID (Product ID) are custom and non standard/registered. The Devices
 * only has one configuration (Config #1)
 */
const tusb_desc_device_t descriptorDeviceFullspeed = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,

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

// Enumeration of the different USB Interfaces used by Config #1
enum {
    ITF_NUM_CDC_0 = 0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_TOTAL
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

/**
 * @brief Config #1 Descriptor for USB
 * This is sent as a response to the GET CONFIGURATION Request. It specifies only one CDC Interfaces
 */
uint8_t const descriptorConfigurationFullspeed[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, CONFIG0_NAME, CONFIG_TOTAL_LEN, 0x00, 200),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, INTERFACE0_NAME, EP_NUM_CDC_CONFIG_NOTIF, 8, EP_NUM_CDC_CONFIG_OUT, EP_NUM_CDC_CONFIG_IN, CFG_TUD_CDC_RX_BUFSIZE)
};


// --------------------------------------------------------------------+
// Descriptor Strings
// --------------------------------------------------------------------+

/**
 * @brief This buffer stores the serial Number for later use. 
 * (Only used by the @ref tud_descriptor_string_cb()).
 * This mapped into the @ref stringDescriptorArray
 */
char serialNumberBuffer[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

/**
 * @brief Stores all strings used by USB
 * The Indices are not random but defined by @ref STRING_DESCRIPTOR_INDEX
 */
char const* stringDescriptorArray [] = {
    (const char[]) { 0x09, 0x04 },
    "TinyUSB",
    "OSCI-Vector-Display",
    serialNumberBuffer,
    "Instruction Configuration",
    "Vector Data",
};





