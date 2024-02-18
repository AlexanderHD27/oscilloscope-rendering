#include "pico/types.h"
#include "pico/stdlib.h"

#include "tusb.h"
#include "tusb_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "dac.h"

void usb_main_task() {
    vTaskDelay(50);
    tusb_init();

    while (true) {
        tud_task();
        vTaskDelay(100);
    }
}

void usb_rx_task() {
    vTaskDelay(100);

    uint32_t bytes_available;
    static uint8_t * rx_buffer = NULL; 
    rx_buffer = dac_acquireInstructionBufferPointer();
    
    while (true) {
        vTaskDelay(100);

        bytes_available = tud_cdc_n_available(0);
        if (bytes_available) {
            gpio_put(25, 1);
            uint32_t count = tud_cdc_n_read(0, rx_buffer, 64);
            dac_submitInstructions(rx_buffer, count);
            gpio_put(25, 0);
        } else {
            
        }

    }
    
}

void tud_mount_cb(void) {
    
}

void tud_umount_cb(void) {
    
}

void tud_suspend_cb(bool remote_wakeup_en) {

}

void tud_resume_cb(void) {

}
