#include "pico/types.h"
#include "pico/stdlib.h"

#include "tusb.h"
#include "tusb_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "includeGlobals.h"
#include "dac.h"

bool rx_ins_ready = true;

void usb_main_task() {
    vTaskDelay(10);
    tusb_init();

    switch (usbInputMode) {
    case RAW:
        usb_main_raw_mode();
        break;

    case INSTRUCTION:
        usb_main_instruction_mode();
        break;
    
    default:
        break;
    }
}

void usb_main_instruction_mode() {
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

void usb_main_raw_mode() {
    uint32_t bytes_recv = 0;
    uint32_t bytes_available;

    frameBuffer_t new_frame_buffer;
    while (!xQueueReceive(g_unusedFrameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    size_t size = new_frame_buffer.size*sizeof(uint16_t)*2;

    while (true) {
        tud_task();

        bytes_available = tud_cdc_n_available(0);
        
        if(bytes_available) {
            gpio_put(25, 1);
            bytes_recv += tud_cdc_n_read(0, &((uint8_t *)(new_frame_buffer.buffer))[bytes_recv], size - bytes_recv);
            printf("[USB] rx %d/%d bytes\n", bytes_recv, size);

            if(bytes_recv >= size) {
                // Buffer filled, we can sent it to the queue
                while(!xQueueSend(g_frameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
                // And get a new one
                while (!xQueueReceive(g_unusedFrameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
                size = new_frame_buffer.size*sizeof(uint16_t)*2;
                bytes_recv = 0;
                printf("new buffer");

            }
            
            gpio_put(25, 0);
        }

        if(!bytes_available || bytes_recv == 0)
            vTaskDelay(pdMS_TO_TICKS(10));
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
