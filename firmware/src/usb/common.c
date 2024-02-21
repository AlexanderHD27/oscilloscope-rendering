#include "pico/types.h"
#include "pico/stdlib.h"

#include "tusb.h"
#include "tusb_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "dac.h"

bool rx_ins_ready = true;

void usb_main_task() {
    vTaskDelay(10);
    tusb_init();

    uint32_t bytes_available;
    static uint8_t * rx_buffer = NULL; 
    rx_buffer = dac_acquireInstructionBufferPointer();
    
    while (true) {
        tud_task();
        
        // RX Buffer
        bytes_available = tud_cdc_n_available(0);
        
        if (bytes_available) {
            rx_ins_ready = false;
            gpio_put(25, 1);

            // Read from USB, Submit Buffer, Acquire New One
            uint32_t count = tud_cdc_n_read(0, rx_buffer, 64);
            //dac_submitInstructions(rx_buffer, count);
            //rx_buffer = dac_acquireInstructionBufferPointer();

            gpio_put(25, 0);
            // Set Flag, for HID Interface, that Data can be written again!
            rx_ins_ready = true;

            // Send Report
            //tud_hid_report(0, &rx_ins_ready, 1);
        }

        //vTaskDelay(1);
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
