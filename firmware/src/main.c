#include <stdio.h>
#include <stdlib.h>

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

#define BUFFER_SIZE 0x1000
static uint16_t output_buffer_a [BUFFER_SIZE*2];
static uint16_t output_buffer_b [BUFFER_SIZE*2];

volatile const uint16_t (* output_buffer_a_ptr)[] = &output_buffer_a;
volatile const uint16_t (* output_buffer_b_ptr)[] = &output_buffer_a;

void handle_next_frame(BUF_SEL empty_buf) {
    if(empty_buf == A) {

    } else if(empty_buf == B) {

    }
}

int main() {

    gen_square_wave(output_buffer_a, BUFFER_SIZE);

    gen_rect(output_buffer_b, BUFFER_SIZE, 
        CONV_NORMAL_TO_UINT16(-0.25), CONV_NORMAL_TO_UINT16(-0.25), 
        CONV_NORMAL_TO_UINT16(+1.00), CONV_NORMAL_TO_UINT16(+1.00)
    );

    const uint32_t gpio_out_mask = (1 << LED) | (1 << LED_GREEN) | (1 << LED_YELLOW);
    gpio_init_mask(gpio_out_mask);
    gpio_set_dir_masked(gpio_out_mask, gpio_out_mask);

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    init_dac_driver(pio, sm,
        DATA_PIN_START, CTRL_PIN_START,
        output_buffer_a_ptr, output_buffer_b_ptr, BUFFER_SIZE,
        handle_next_frame
    );

    while (true) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }
}
