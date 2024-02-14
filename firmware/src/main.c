#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "hardware/pio.h"
#include "dac.h"

#define DATA_PIN_START 2
#define CTRL_PIN_START 10

// Pin Layout:
// 2-9  Parallel Data
//   10 Enable_Pre_Load
//   11 Sel_A
//   12 Sel_B
//   13 LD_Master

#define LED 25
#define LED_GREEN 17
#define LED_YELLOW 16

uint32_t counter = 0;

void provide_job_task(void * param) {
    while (true) {
        uint8_t * __ins_buf = acquire_instruction_buffer_pointer();
        uint8_t * ins_buf = __ins_buf;
        ins_buf += add_ins_sine(ins_buf, X, BUFFER_SIZE/2, 0x00a0, 0xff60, (uint16_t)(BUFFER_SIZE/2/3), BUFFER_SIZE/2/4);
        ins_buf += add_ins_sine(ins_buf, Y, BUFFER_SIZE/2, 0x00a0, 0xff60, (uint16_t)(BUFFER_SIZE/2/2), 0x0000);
        ins_buf += add_ins_const(ins_buf, X, BUFFER_SIZE/2, 0x7fff - 0xffff*(0.5*cosf( PI*((float)(counter*3)/(BUFFER_SIZE/2-1)))) );
        ins_buf += add_ins_const(ins_buf, Y, BUFFER_SIZE/2, 0x7fff - 0xffff*(0.5*sinf( PI*((float)(counter*2)/(BUFFER_SIZE/2-1)))) );
counter = (counter + 0x1f) % BUFFER_SIZE;
        submit_instructions(__ins_buf, ins_buf - __ins_buf);
    }
}


int main() {

    // Init Debug LEDs
    const uint32_t gpio_out_mask = (1 << LED) | (1 << LED_GREEN) | (1 << LED_YELLOW);
    gpio_init_mask(gpio_out_mask);
    gpio_set_dir_masked(gpio_out_mask, gpio_out_mask);
    gpio_put(16, true);

    // Claim PIO & sm
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);

    // Init DAC
    init_dac(pio, sm, DATA_PIN_START, CTRL_PIN_START);

    // Create Test Task for providing a constant stream of instruction
    TaskHandle_t provide_job_task_handle;
    xTaskCreate(provide_job_task, "[Test] Create Jobs", 128, NULL, 1, &provide_job_task_handle);

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();
}
