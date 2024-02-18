#include "pico/types.h"
#include "pico/stdlib.h"

#include "tusb.h"
#include "tusb_config.h"

#include "FreeRTOS.h"
#include "task.h"

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
    while (true) {
        vTaskDelay(100);
        uint8_t buf[64];

        if ( tud_cdc_n_available(0)) {
            uint32_t count = tud_cdc_n_read(0, buf, sizeof(buf));
            gpio_put(25, 1);
        }

    }
    
}

// tiny USB Callback

void tud_mount_cb(void) {
    
}

void tud_umount_cb(void) {
    
}

void tud_suspend_cb(bool remote_wakeup_en) {

}

void tud_resume_cb(void) {

}


TU_ATTR_WEAK void tud_vendor_rx_cb(uint8_t itf) {
    gpio_put(25, 1);
};

TU_ATTR_WEAK void tud_vendor_tx_cb(uint8_t itf, uint32_t sent_bytes) {

};
