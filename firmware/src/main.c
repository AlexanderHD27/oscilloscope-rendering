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
static uint16_t output_buffer [BUFFER_SIZE*2];
volatile const uint16_t (* output_buffer_ptr)[] = &output_buffer;

void handle_next_frame(enum BUF_SEL empty_buf) {

}

int main() {

    gen_calibration_cross(output_buffer, BUFFER_SIZE);

    const uint32_t gpio_out_mask = (1 << LED) | (1 << LED_GREEN) | (1 << LED_YELLOW);
    gpio_init_mask(gpio_out_mask);
    gpio_set_dir_masked(gpio_out_mask, gpio_out_mask);

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    init_dac_driver(pio, sm,
        DATA_PIN_START, CTRL_PIN_START,
        output_buffer_ptr, output_buffer_ptr, BUFFER_SIZE,
        handle_next_frame
    );

    while (true) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }
}
