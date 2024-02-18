/**
 * @file main.c
 * @author AlexanderHD27 (you@domain.com)
 * @brief Main file of the RPi Pico Firmware for the Oscilloscope-Renderer
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "hardware/pio.h"
#include "dac.h"

#define PIN_DATA_START 2
#define PIN_CTRL_START 10

// Pin Layout:
// 2-9  Parallel Data
//   10 Enable_Pre_Load
//   11 Sel_A
//   12 Sel_B
//   13 LD_Master

#define PIN_LED 25
#define PIN_LED_GREEN 17
#define PIN_LED_YELLOW 16

uint32_t counter = 0;

void provideJobTask(void * param) {
    while (true) {
        uint8_t * _insBuf = dac_acquireInstructionBufferPointer();
        uint8_t * insBuf = _insBuf;
        insBuf += gen_addInstructionSine(insBuf, X, BUFFER_SIZE/2, 0x00a0, 0xff60, (uint16_t)(BUFFER_SIZE/2/3), BUFFER_SIZE/2/4);
        insBuf += gen_addInstructionSine(insBuf, Y, BUFFER_SIZE/2, 0x00a0, 0xff60, (uint16_t)(BUFFER_SIZE/2/2), 0x0000);
        insBuf += gen_addInstructionConst(insBuf, X, BUFFER_SIZE/2, 0x7fff - 0xffff*(0.5*cosf( PI*((float)(counter*3)/(BUFFER_SIZE/2-1)))) );
        insBuf += gen_addInstructionConst(insBuf, Y, BUFFER_SIZE/2, 0x7fff - 0xffff*(0.5*sinf( PI*((float)(counter*2)/(BUFFER_SIZE/2-1)))) );
counter = (counter + 0x1f) % BUFFER_SIZE;
        dac_submitInstructions(_insBuf, insBuf - _insBuf);
    }
}


int main() {

    // Init Debug LEDs
    const uint32_t gpioOutMask = (1 << PIN_LED) | (1 << PIN_LED_GREEN) | (1 << PIN_LED_YELLOW);
    gpio_init_mask(gpioOutMask);
    gpio_set_dir_masked(gpioOutMask, gpioOutMask);
    gpio_put(16, true);

    // Claim PIO & sm
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);

    // Init DAC
    dac_init(pio, sm, PIN_DATA_START, PIN_CTRL_START);

    // Create Test Task for providing a constant stream of instruction
    //TaskHandle_t provideJobTaskHandle;
    //xTaskCreate(provideJobTask, "[Test] Create Jobs", 128, NULL, 1, &provideJobTaskHandle);
    //vTaskCoreAffinitySet(provideJobTaskHandle, (1 << 0));

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();

    while(1){};
}
