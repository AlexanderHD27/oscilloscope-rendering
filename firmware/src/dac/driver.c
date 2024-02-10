#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "hardware/timer.h"

#include "dac.pio.h"
#include "dac.h"

#include "dac.h"

int dma_chan;
uint16_t (* out_buffer_a)[];
uint16_t (* out_buffer_b)[];
enum BUF_SEL buffer_in_use;
size_t buf_size;

void (*__next_frame_handler)(enum BUF_SEL) = NULL;

/**
 * Called every time a frame is finished
 * It acks the irq, set the new buffer for the dma-channel to read, flips the current used buffer marker
 * This should be set as isr for dma
*/ 
void __isr_dma() {
    dma_channel_acknowledge_irq0(dma_chan);
    dma_channel_transfer_from_buffer_now(dma_chan, *(buffer_in_use == A ? out_buffer_b : out_buffer_a), buf_size);

    if(__next_frame_handler)
        __next_frame_handler(buffer_in_use);

    buffer_in_use = buffer_in_use == A ? B : A;
}

/**
 * 
 * @brief Configures and starts PIO State machines to create the signal for the DACs.
 * This driver use one PIO core and one State machine
 *
 * Pin Layout:
 *      Data Pins: 0-7 (Reverse order)
 *      Control Pins:
 *          0 pre-register LE
 *          1 Sel_A (High or Lower 8 bits) Inverted
 *          2 Sel_B (X or Y Channel) Inverted
 *          3 Master LE
 * 
 * 
 * Buffer is layout as { X0, Y0, X1, X1, ... }
 * This means that a buffer needs to have size of 2*buffer_size (buffer_size is referring to the number of samples it has)
 * The buffer is not passed directly, but as pointer to the array (uint16_t (* buffer_a)[]), such that the buffer can be swap
 * for another without coping data around. Or put this: the pointer to the pointer to the data is a passed
 * The Buffer pointers are store in a global variable, so make sure that these pointers do not go out of scope
 * 
 * @param pio either pio0 or pio1
 * @param sm pio state machine used
 * @param control_pin_start starting offset for the control pins (in total 4 control bins are used)
 * @param data_pin_start starting offset for data connection (in total 8 data pins are used)
 * @param buffer_a pointer to output buffer A (the buffer needs to be of size 2*buffer_size)
 * @param buffer_b pointer to output buffer B (the buffer needs to be of size 2*buffer_size)
 * @param buffer_size who many X-Y points are contained in a frame
 * @param next_frame_handler Handler function, called after a frame swap occurred. Its parsed the used buffer, to modify/fill. This function should talk only short amount of time to execute
*/
void init_dac_driver(PIO pio, uint sm, 
    uint data_pin_start, uint control_pin_start, 
    uint16_t (*buffer_a)[], uint16_t (*buffer_b)[], size_t buffer_size,
    void (*next_frame_handler )(enum BUF_SEL)
) {
    __next_frame_handler = next_frame_handler;
    out_buffer_a = buffer_a;
    out_buffer_b = buffer_b;
    buf_size = buffer_size;
    buffer_in_use = A;

    // Configure PIOs
    {
        const uint pio_program_offset = pio_add_program(pio, &dac_program);
        pio_sm_config pio_config = dac_program_get_default_config(pio_program_offset);

        // Clock Div
        // Frequency can be from 2000 Hz to 133 MHz 
        // 20MHz Frequency / 20 cycles per sample =  1 MSamples per Second
        const float freq = 2 * 1000000;
        const float clock_divider = ((float)(SYS_CLK_KHZ * 1000) / (freq))/2;
        sm_config_set_clkdiv(&pio_config, clock_divider);

        // Config FIFOs
        sm_config_set_fifo_join(&pio_config, PIO_FIFO_JOIN_TX);
        sm_config_set_out_shift(&pio_config, true, true, 32); // Auto PULL

        // Applying PIO config
        pio_sm_init(pio, sm, pio_program_offset, &pio_config);

        // Set Up Pins
        for(int i = 0; i<12; i++) {
            pio_gpio_init(pio, data_pin_start + i);
        }

        pio_sm_set_consecutive_pindirs(pio, sm, data_pin_start, 8, true);
        pio_sm_set_consecutive_pindirs(pio, sm, control_pin_start, 4, true);
        
        pio_sm_set_out_pins(pio, sm, data_pin_start, 8);
        pio_sm_set_set_pins(pio, sm, control_pin_start, 4);
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
            (void*)(*out_buffer_a), buffer_size, true
        );

        irq_set_exclusive_handler(DMA_IRQ_0, __isr_dma);
        dma_channel_set_irq0_enabled(dma_chan, true);
        irq_set_enabled(DMA_IRQ_0, true);
    }

    // starting SMs
    pio_sm_set_enabled(pio, sm, true);
}
