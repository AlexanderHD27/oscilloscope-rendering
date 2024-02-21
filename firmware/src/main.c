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

#include <stdlib.h>
#include <math.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "dac.h"

#include <stdio.h>

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

int main() {
    stdio_uart_init();
    printf("\n\n======== Reset ========\n");

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

    // Start FreeRTOS Scheduler
    printf("[SYS] Run Schedular\n");
    vTaskStartScheduler();

    while(1){};
}
