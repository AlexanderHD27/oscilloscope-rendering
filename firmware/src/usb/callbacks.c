#include "pico/stdlib.h"
#include "pico/unique_id.h"

#include "tusb.h"
#include "usb.h"

extern tusb_desc_device_t const descriptorDeviceFullspeed;
extern uint8_t const descriptorConfigurationFullspeed[];

extern char const* stringDescriptorArray [];
extern char serialNumberBuffer[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

/**
 * @brief Static buffer used by @ref tud_descriptor_string_cb to responded with string
 */
static uint16_t descriptorStringBuffer[32];

/**
 * @brief Callback, invoked on USB GET STRING DESCRIPTOR request.
 * This is invoked by tinyusb. Should return a pointer to a String descriptor (aka. char[]). The String 
 * should no be longer then 32 chars
 * 
 * @param index What String form @ref stringDescriptorArray to return
 * @param langid Language of the string (We No Speak Americano)
 * @return uint16_t const* This should point to char array, the first 2 bytes should be the length of the array. See USB Spec
 */
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    
    uint8_t charCount = 0;
    const char * string;

    switch (index) {
    case LANG_ID:
        memcpy(&descriptorStringBuffer[1], stringDescriptorArray[0], 2);
        charCount = 1;
        break;

    case SERIAL_NUMBER:
        pico_get_unique_board_id_string(serialNumberBuffer, sizeof(serialNumberBuffer));
    case VENDOR_NAME:
    case PRODUCT_NAME:
    case CONFIG0_NAME:
    case INTERFACE0_NAME:
        string = stringDescriptorArray[index];
        charCount = MIN(strlen(string), 32);

        for(uint8_t i=0; i<charCount; i++)
            descriptorStringBuffer[i+1] = string[i];

        break;

    default:
        return NULL;
    }
    
    descriptorStringBuffer[0] = (TUSB_DESC_STRING << 8 ) | (2*charCount + 2);
    return descriptorStringBuffer;
}

/**
 * @brief Callback, invoked on USB GET DEVICE DESCRIPTOR request.
 * This is invoked by tinyusb. Should return a pointer to the @ref descriptorDeviceFullspeed
 * 
 * @return uint8_t const* Pointer to the configuration, in this case @ref descriptorDeviceFullspeed
 */
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &descriptorDeviceFullspeed;
}

/**
 * @brief Callback, invoked on USB GET CONFIGURATION DESCRIPTOR request
 * This is invoked by tinyusb. 
 * 
 * @param index What Configuration should be returned: We only have on so, we ignore it
 * @return uint8_t const* Pointer to the correct configuration Array. We only have one config, so always return @ref descriptorConfigurationFullspeed
 */
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
  (void) index; // for multiple configurations
  return descriptorConfigurationFullspeed;
}
