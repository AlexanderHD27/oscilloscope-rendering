#include "pico/stdlib.h"
#include "pico/unique_id.h"

#include "tusb.h"
#include "usb.h"

extern tusb_desc_device_t const descriptorDeviceFullspeed;
extern uint8_t const descriptorConfigurationFullspeed[];

extern char const* stringDescriptorArray [];
extern char serialNumberBuffer[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

static uint16_t descriptorStringBuffer[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    
    uint8_t charCount = 0;
    char * string;

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


// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &descriptorDeviceFullspeed;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
  (void) index; // for multiple configurations
  return descriptorConfigurationFullspeed;
}
