#include "pico/stdlib.h"

#include "dac.h"
#include "gen.h"
#include "includeGlobals.h"

void gen_processingTaskFunction(void * param) {
    instructionBuffer_t new_instruction_buffer;
    frameBuffer_t new_frame_buffer;

    while (true) {
        // Wait until instruction are available
        if(xQueueReceive(g_instructionBufferQueue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {

            // Get valid frame buffer
            while (!xQueueReceive(g_unusedFrameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
            

            gpio_put(17, true);
            gen_processInstruction(new_instruction_buffer, new_frame_buffer);
            gpio_put(17, false);

            // Send resulting buffer to PIO
            while(!xQueueSend(g_frameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}

            // Returning Instruction buffer
            while(!xQueueSend(g_unusedInstructionBufferQueue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
        }
    }
}

