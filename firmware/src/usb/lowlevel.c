#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/irq.h"
#include "hardware/resets.h"

#include <string.h>
#include <stdio.h>

#include "hardware/resets.h"
#include "hardware/structs/usb.h"

#include "FreeRTOS.h"
#include "task.h"

#include "usb.h"
#include "usb_common.h"
#include "usb_descriptors.h"
#include "usb_globals.h"

#define usb_hw_set ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

// Global USB State

/**
 * @brief Usb devices address set by the host
 * Do not use this until @ref addressSet is true
 */
uint8_t deviceAddress = 0;

/**
 * @brief Indicates if address was set by the host
 */
bool addressSet = false;

/**
 * @brief Indicates if a configuration was set by the host
 */
bool configured = false;

// ISRs

void isr_usbCTRL() {
    uint32_t status = usb_hw->ints;
    uint32_t handled = 0;

    // Handling Package SETUP Interrupt
    if(status & USB_INTS_SETUP_REQ_BITS) {
        handled |= USB_INTS_SETUP_REQ_BITS;
        usb_hw_clear->sie_status = USB_SIE_STATUS_SETUP_REC_BITS;
        usb_handleSetupPackage();
    }

    // Handling RESET BUS Interrupt
    if(status & USB_INTS_BUS_RESET_BITS) {
        handled |= USB_INTS_BUS_RESET_BITS;
        usb_hw_clear->sie_status = USB_SIE_STATUS_BUS_RESET_BITS;
        usb_handleBusReset();
    }

    // Status BUFF
    if(status & USB_INTS_BUFF_STATUS_BITS) {
        handled |= USB_INTS_BUFF_STATUS_BITS;
        usb_handleBuffersDone();
    }

    if (status ^ handled) {
        panic("Unhandled IRQ 0x%x\n", (uint) (status ^ handled));
    }
}

extern TaskHandle_t usbSetupTaskHandle;

void usb_initTaskFunction(void * param) {
    printf("[USB] Init started\n");
    // Reseting USB and clearing dpram
    reset_block(RESETS_RESET_USBCTRL_BITS);
    unreset_block_wait(RESETS_RESET_USBCTRL_BITS);

    memset(usb_dpram, 0, sizeof(usb_dpram));

    // Enable IRQs & Registering isr
    irq_set_enabled(USBCTRL_IRQ, true);
    irq_set_exclusive_handler(USBCTRL_IRQ, isr_usbCTRL);

    // Config USB Mux to Device
    usb_hw->muxing = USB_USB_MUXING_SOFTCON_BITS | USB_USB_MUXING_TO_PHY_BITS;
    usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;

    usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS;

    // Enables USB Interrupts for buffer done, bus reset and setup, packages
    usb_hw->inte = USB_INTS_BUFF_STATUS_BITS | USB_INTS_BUS_RESET_BITS | USB_INTS_SETUP_REQ_BITS;

    usb_configEndpoints();

    // Set pull up to enable fullspeed
    usb_hw_set->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;

    printf("[USB] Init done\n");
    vTaskDelete(NULL);
}

void usb_configEndpoints() {
    for(int i=0; i<NUM_ENDPOINTS; i++) {
        if(deviceData.endpoints[i].descriptor)
            usb_configEndpointFromConfig(&deviceData.endpoints[i]);
    }
    printf("[USB] Configured Endpoints");
}

void usb_configEndpointFromConfig(const struct usb_endpointConfiguration * config) {
    printf("[USB] Setup Endpoint 0x%x\n", config->descriptor->bEndpointAddress);

    if(config->bufferCTRL)
        return; // This only should happen to ep0


}

void usb_reset() {
    deviceAddress = 0;
    addressSet = false;
    configured = false;

    usb_hw->dev_addr_ctrl = 0;
}