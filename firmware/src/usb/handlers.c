#include <string.h>

#include "hardware/resets.h"
#include "hardware/structs/usb.h"

#include "usb_globals.h"
#include "usb_common.h"
#include "usb_descriptors.h"
#include "usb.h"

#define usb_hw_set ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

void usb_handleBusReset() {
    printf("[USB] Interrupt RESET\n");
    usb_reset();
}

void usb_handlePlugIn() {
    printf("[USB] Plugged In");
}

void usb_handlePlugOut() {
    printf("[USB] Disconnected");
}

void usb_handleSetupPackage() {
    volatile usb_setupPacket_t * pkt = (usb_setupPacket_t *) &usb_dpram->setup_packet;
    uint8_t dir  = pkt->bmRequestType & 0x80;
    uint8_t type = pkt->bmRequestType & 0x60;

    printf("[USB] Interrupt (%x, %x) SETUP %s ", pkt->bmRequestType, pkt->bRequest, dir ? "IN" : "OUT");

    if(dir == USB_DIR_IN) {
        // Read data in from the devices (expects data respones)

        switch(type) {
            case Standard: {
                switch (pkt->bRequest) {
                case GET_DESCRIPTOR:
                    printf(": GET_DESCRIPTOR");
                    usb_handleGetDeviceDescriptor(pkt);
                    break;
                
                case GET_CONFIGURATION:
                    printf(": GET_CONFIGURATION");
                    break;

                case GET_INTERFACE:
                    printf(": GET_INTERFACE");
                    break;

                case GET_STATUS:
                    printf(": GET_STATUS");
                    break;

                case SYNCH_FRAME:
                    printf(": SYNCH_FRAME");
                    break;

                default:
                    break;
                }
            }
                break;

            case Class:
                break;
            case Vendor:
                break;
            case Reserved:
                break;
        }

    } else if (dir == USB_DIR_OUT) {
        switch(type) {
            case Standard: {
                switch (pkt->bRequest) {
                case SET_ADDRESS:
                    printf(": SET_ADDRESS");
                    deviceAddress = pkt->wValue & 0xff;
                    addressSet = true;
                    usb_ackOutRequest();
                    printf(" (%d)", deviceAddress);
                    break;

                case SET_CONFIGURATION:
                    printf(": SET_CONFIGURATION");
                    break;

                case SET_DESCRIPTOR:
                    printf(": SET_DESCRIPTOR");
                    break;

                case SET_FEATURE:
                    printf(": SET_FEATURE");
                    break;

                case SET_INTERFACE:
                    printf(": SET_INTERFACE");
                    break;

                case CLEAR_FEATURE:
                    printf(": CLEAR_FEATURE");
                    break;

                default:
                    break;
                }
            }
                break;
        
            case Class:
                break;
            case Vendor:
                break;
            case Reserved:
                break;
        }
    }
    
    printf("\n");
}  


void usb_handleBufferDone(uint8_t endpointNumber, bool in) {
    uint8_t addr = endpointNumber | (in ? USB_DIR_IN : 0);

    printf("[USB] EP %d %s ", endpointNumber, in ? "IN" : "OUT");

    for(uint i=0; i < NUM_ENDPOINTS; i++) {
        struct usb_endpointConfiguration * endpoint = &deviceData.endpoints[i];
        if(endpoint->descriptor && endpoint->descriptor->bEndpointAddress == addr) {
            return;
        }
    }
}

void usb_handleBuffersDone() {
    printf("[USB] Interrupt Buffer Done\n");

    uint32_t buffers = usb_hw->buf_status;
    uint32_t remainingBuffers = buffers;

    uint32_t selectBufferBit = 1;

    for(int i=0; remainingBuffers && i < NUM_ENDPOINTS * 2; i++) {
        if(remainingBuffers & selectBufferBit) {
            usb_hw_clear->buf_status = selectBufferBit;

            // IN transfer for even i, OUT transfer for odd i
            usb_handleBufferDone(i >> 1, !(i & 0b1));

            remainingBuffers &= ~selectBufferBit; // Remove Bit from remaining Buffer
        }
        
        selectBufferBit <<= 1;
    }
}

/**
 * @brief Handles a USB GET DEVICE request, by responding with the device descriptor
 */
void usb_handleGetDeviceDescriptor(volatile struct usb_setupPacket *pkt) {    
    struct usb_endpointConfiguration * endpoint0 = usb_getEndpointByAddress(EP0_IN_ADDR);
    endpoint0->nextPid = 1; // Always response with PID 0

    usb_startTransfer(endpoint0, (uint8_t *) deviceData.deviceDesc, MIN(pkt->wLength, sizeof(struct usb_deviceDescriptor)));
}

/**
 * @brief Given an endpoint configuration, returns true if the endpoint
 * is transmitting data to the host (i.e. is an IN endpoint)
 * @author from the RRi Pico SDK dev_lowlevel example
 * 
 * @param ep, the endpoint configuration
 * @return true
 * @return false
 */
static inline bool isEndpointTX(struct usb_endpointConfiguration *ep) {
    return ep->descriptor->bEndpointAddress & USB_DIR_IN;
}

void usb_startTransfer(struct usb_endpointConfiguration *ep, uint8_t *buf, uint16_t len) {
    assert(len <= 64);

    uint32_t val = len;

    if(isEndpointTX(ep)) {
        memcpy((void *) ep->dataBuffer, (void *) buf, len);
        val |= USB_BUF_CTRL_FULL;
    }

    // Selecting the correct buffer
    val |= ep->nextPid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    ep->nextPid ^= 1u;
    *ep->bufferCTRL = val;

    // This is done (setting the available bit separate from the rest)
    // is done because of different clocks speed of the CPU and SIE
    // See https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf page 384 (4.1.2.5.1 Concurrent access)
    __asm__(
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
    );

    *ep->bufferCTRL |= USB_BUF_CTRL_AVAIL;
}

extern struct usb_deviceData deviceData;

struct usb_endpointConfiguration * usb_getEndpointByAddress(uint8_t addr) {
    for(int i=0; i<NUM_ENDPOINTS; i++) {
        if(deviceData.endpoints[i].descriptor != NULL && deviceData.endpoints[i].descriptor->bEndpointAddress == addr) {
            return &deviceData.endpoints[i];
        }
    }

    return NULL;
}

void usb_ackOutRequest(void) {
    usb_startTransfer(usb_getEndpointByAddress(EP0_IN_ADDR), NULL, 0);
}
