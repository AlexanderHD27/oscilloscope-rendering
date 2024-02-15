#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "hardware/timer.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "dac.pio.h"
#include "dac.h"

#include "includeGlobals.h"

int dma_chan;

// Output Buffer should never be touched besides the ISR
frame_buffer_t current_output_buffer; 
frame_buffer_t next_output_buffer;

/**
 * Called every time a frame is finished
 * It acks the irq, set the new buffer for the dma-channel to read, flips the current used buffer marker
 * This should be set as isr for dma
*/ 
void __isr_dma() {
    static uint repeat_counter = 0;

    if(repeat_counter == 0) { // Every FRAME_REPEATS cycles, the frame is updated
        // Refuel the DMA with the new buffer as soon as the isr triggers
        dma_channel_acknowledge_irq0(dma_chan);
        dma_channel_transfer_from_buffer_now(dma_chan, *(next_output_buffer.buffer), next_output_buffer.size);

        BaseType_t xTaskWokenByReceive = pdFALSE;    

        // Swapping around buffers (Queue <- current_output_buffer <- next_output_buffer)
        frame_buffer_t old_output_buffer = current_output_buffer; 
        current_output_buffer = next_output_buffer;

        // If old buffers is used, don't send it back
        if(current_output_buffer.buffer != old_output_buffer.buffer) {
            while(!xQueueSendFromISR(unused_frame_buffer_queue, &old_output_buffer, &xTaskWokenByReceive));
            // This should never block
        }

        // Try to recv the next buffer
        frame_buffer_t frame_buffer;

        if(xQueueReceiveFromISR(frame_buffer_queue, &frame_buffer, &xTaskWokenByReceive)) {
            gpio_put(16, false);
            // Got new buffer! -> recv buffer will be the next buffer
            next_output_buffer = frame_buffer;
        } else {
            gpio_put(16, true);
            // buffer miss: repeat the old buffer and check next cycle
            repeat_counter = FRAME_REPEAT-1;
        }
    } else {
        dma_channel_acknowledge_irq0(dma_chan);
        dma_channel_transfer_from_buffer_now(dma_chan, *(current_output_buffer.buffer), current_output_buffer.size);
    }

    repeat_counter = (repeat_counter + 1) % FRAME_REPEAT;
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
void _dac_initPIO(PIO pio, uint sm, uint data_pin_start, uint control_pin_start) {
    // Get initial buffers
    {
        frame_buffer_t first_buf;
        first_buf.size = BUFFER_SIZE;
        first_buf.buffer = &main_frame_buffers[0];
        current_output_buffer = first_buf;
        next_output_buffer = first_buf;
    }

    // Configure PIOs
    {
        const uint pio_program_offset = pio_add_program(pio, &dac_program);
        pio_sm_config pio_config = dac_program_get_default_config(pio_program_offset);

        // Clock Div
        const float freq = PIO_FREQ_KHZ * 1000;
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
            &main_frame_buffers[0], sizeof(main_frame_buffers[0])/2, true
        );

        irq_set_exclusive_handler(DMA_IRQ_0, __isr_dma);
        dma_channel_set_irq0_enabled(dma_chan, true);
        irq_set_enabled(DMA_IRQ_0, true);
    }

    // starting SMs
    pio_sm_set_enabled(pio, sm, true);
}
