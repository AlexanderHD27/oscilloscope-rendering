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
    instructionBufferClusters_t insBuffer = dac_acquireInstructionBuffer();
    uint i = 0;
    
    while (true) {
        tud_task();
        
        // RX Buffer
        bytes_available = tud_cdc_n_available(0);
        
        if (bytes_available) {
            rx_ins_ready = false;
            gpio_put(25, 1);

            // Read from USB into buffer/cluster
            insBuffer.size[i] = tud_cdc_n_read(0, (insBuffer.buffer + i*INSTRUCTION_BUF_SIZE), 64);
            tud_hid_report(0, &rx_ins_ready, 1);
            printf("[USB] RX Cluster %d (%d)\n", i, insBuffer.size[i]);

            i++;
            if(i >= INSTRUCTION_CLUSTER_SIZE) {
                // if Cluster is full, we:  Submit Buffer, Acquire New One
                i = 0;
                dac_submitInstructions(insBuffer);
                insBuffer = dac_acquireInstructionBuffer();
                printf("Submit buffer!\n");
                vTaskDelay(10);
            }

            gpio_put(25, 0);
            // Set Flag, for HID Interface, that Data can be written again!
            rx_ins_ready = true;
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
