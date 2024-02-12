#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

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

static uint16_t output_buffer_a [BUFFER_SIZE*2];
static uint16_t output_buffer_b [BUFFER_SIZE*2];

uint16_t (* output_buffer_a_ptr)[] = &output_buffer_a;
uint16_t (* output_buffer_b_ptr)[] = &output_buffer_a;

int main() {

    //pregen_square_wave(output_buffer_a, BUFFER_SIZE);

    pregen_rect(output_buffer_a, BUFFER_SIZE, 
        CONV_NORMAL_TO_UINT16(-1.00), CONV_NORMAL_TO_UINT16(-1.00),
        CONV_NORMAL_TO_UINT16(+0.25), CONV_NORMAL_TO_UINT16(+0.25) 
    );

    pregen_rect(output_buffer_b, BUFFER_SIZE, 
        CONV_NORMAL_TO_UINT16(-0.25), CONV_NORMAL_TO_UINT16(-0.25), 
        CONV_NORMAL_TO_UINT16(+1.00), CONV_NORMAL_TO_UINT16(+1.00)
    );

    const uint32_t gpio_out_mask = (1 << LED) | (1 << LED_GREEN) | (1 << LED_YELLOW);
    gpio_init_mask(gpio_out_mask);
    gpio_set_dir_masked(gpio_out_mask, gpio_out_mask);

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    init_dac(pio, sm,
        DATA_PIN_START, CTRL_PIN_START
    );

    vTaskStartScheduler();
}
