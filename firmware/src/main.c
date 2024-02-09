#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "hardware/timer.h"

#include "dac.pio.h"

#define PI 3.14159265359

#define PIO_CLK_DIV(freq) (SYS_CLK_MHZ*1000000)/(freq)

// FIXME: Added Correct PIN numbers
#define DATA_PIN_START 2
#define OFFSET_LD 0

// Pin Layout:
// 2-9  Parallel Data
//   10 Enable_Pre_Load
//   11 Sel_A
//   12 Sel_B
//   13 LD_Master

#define LED 25

#define BUFFER_SIZE 0x1000

uint16_t output_buffer [BUFFER_SIZE*2];

int dma_chan;

void isr_dma() {
    dma_channel_acknowledge_irq0(dma_chan);
    dma_channel_transfer_from_buffer_now(dma_chan, output_buffer, BUFFER_SIZE);
}

static void init_dac_driver(PIO pio, uint sm) {
    // Configure PIOs
    {
        const uint pio_program_offset = pio_add_program(pio, &dac_program);
        pio_sm_config pio_config = dac_program_get_default_config(pio_program_offset);

        // Clock Div
        // Frequency can be from 2000 Hz to 133 MHz 
        // 20MHz Frequency / 20 cylcles per sample =  1 MSamples per Second
        const float freq = 2 * 1000000;
        const float clock_divider = ((float)(SYS_CLK_KHZ * 1000) / (freq))/2;
        sm_config_set_clkdiv(&pio_config, clock_divider);

        // Config Fifos
        sm_config_set_fifo_join(&pio_config, PIO_FIFO_JOIN_TX);
        sm_config_set_out_shift(&pio_config, true, true, 32); // Auto PULL

        // Appling PIO config
        pio_sm_init(pio, sm, pio_program_offset, &pio_config);

        // Set Up Pins
        for(int i = 0; i<12; i++) {
            pio_gpio_init(pio, DATA_PIN_START + i);
        }

        pio_sm_set_consecutive_pindirs(pio, sm, DATA_PIN_START, 12, true);
        
        pio_sm_set_out_pins(pio, sm, DATA_PIN_START, 8);
        pio_sm_set_set_pins(pio, sm, DATA_PIN_START + 8 + OFFSET_LD, 4);
    }

    // Configure DMA
    {
        dma_chan = dma_claim_unused_channel(true);
        dma_channel_config dma_config = dma_channel_get_default_config(dma_chan);
        channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
        channel_config_set_read_increment(&dma_config, true);
        channel_config_set_write_increment(&dma_config, false);
        channel_config_set_dreq(&dma_config, pio_get_dreq(pio, sm, true));

        dma_channel_configure(
            dma_chan, &dma_config,
            &pio->txf[sm], 
            (void*)output_buffer, BUFFER_SIZE, true
        );

        irq_set_exclusive_handler(DMA_IRQ_0, isr_dma);
        dma_channel_set_irq0_enabled(dma_chan, true);
        irq_set_enabled(DMA_IRQ_0, true);
    }


    // starting SMs
    pio_sm_set_enabled(pio, sm, true);
}

int main() {
    volatile uint16_t y0 = 0;
    volatile uint16_t y1 = 0;
    volatile int x_sec = 0;
    volatile int x_sub = 0;

    const uint div = 8;
    const uint sub_x_range = BUFFER_SIZE/div;

    volatile float x_sin = 0.0;

    for(int i=0; i<BUFFER_SIZE; i++) {
        x_sub = i % sub_x_range;
        x_sin = (((float)x_sub)/sub_x_range * 2*PI);
        x_sec = i / sub_x_range;
    
        y0 = (uint16_t)((sinf(x_sin)*0.5 + 0.5) * 0x7fff) + 0xffff/4;
        y1 = (uint16_t)((sinf(x_sin + 0.5*PI)*0.5 + 0.5) * 0x7fff) + 0xffff/4;

        if(x_sec % 2) {
            output_buffer[i*2] = y0;
            output_buffer[i*2 + 1] = y1;
        } else {
            output_buffer[i*2] = x_sec % 8 == 2 ? (((float)x_sub)/sub_x_range) * 0xffff : 0x7fff;
            output_buffer[i*2 + 1] = x_sec % 8 == 0 ? (((float)x_sub)/sub_x_range) * 0xffff : 0x7fff;
        }
  
    }
    

    //for(int i=0; i<BUFFER_SIZE; i++) {
    //    output_buffer[i*2] = (i&0b1) * 0xffff;
    //    output_buffer[i*2 + 1] = ((i&0b1) ^ 0b1) * 0xffff;
    //}

    gpio_init(LED);
    gpio_set_dir(LED, true);

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    init_dac_driver(pio, sm);



    while (true) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }
}
