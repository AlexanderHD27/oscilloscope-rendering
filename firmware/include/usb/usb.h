#include "pico/types.h"
#include "usb_common.h"

#ifndef USB_INCLUDE_HEADER
#define USB_INCLUDE_HEADER

/**
 * @brief Task Function that resets/setups USB Device Controller 
 * 
 */
void usb_initTaskFunction();

/**
 * @brief Configures the endpoints from the endpoints configuration list
 * Empty for now
 */
void usb_configEndpoints();

/**
 * @brief Configures a endpoint based on a endpoint configuration
 * 
 * @param ep pointer to the configuration to apply
 */
void usb_configEndpointFromConfig(const struct usb_endpointConfiguration * ep);

/**
 * @brief Reset USB Connection, but not the onBoard Configuration
 */
void usb_reset();

// Handler Functions

void usb_handleSetupPackage();
void usb_handleBusReset();
void usb_handleBuffersDone();

void usb_handleGetDeviceDescriptor();

struct usb_endpointConfiguration * usb_getEndpointByAddress(uint8_t addr);
void usb_startTransfer(struct usb_endpointConfiguration *ep, uint8_t *buf, uint16_t len);

void usb_handlePlugIn();
void usb_handlePlugOut();

#endif