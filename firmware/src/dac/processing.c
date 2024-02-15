#include "pico/stdlib.h"

#include "dac.h"
#include "gen.h"
#include "includeGlobals.h"

void gen_processingTaskFunction(void * param) {
    instructionBuffer_t new_instruction_buffer;
    frame_buffer_t new_frame_buffer;

    while (true) {
        // Wait until instruction are available
        if(xQueueReceive(instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {

            // Get valid frame buffer
            while (!xQueueReceive(unused_frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
            

            gpio_put(17, true);
            gen_processInstruction(new_instruction_buffer, new_frame_buffer);
            gpio_put(17, false);

            // Send resulting buffer to PIO
            while(!xQueueSend(frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}

            // Returning Instruction buffer
            while(!xQueueSend(unused_instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
        }
    }
}

