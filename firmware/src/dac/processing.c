#include "pico/stdlib.h"

#include "dac.h"
#include "gen.h"
#include "includeGlobals.h"

void gen_processingTaskFunction(void * param) {
    instructionBufferClusters_t new_instruction_buffer;
    frameBuffer_t new_frame_buffer;

    while (true) {
        // Wait until instruction are available
        if(xQueueReceive(g_instructionBufferQueue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {

            
            gpio_put(17, true);
            // Iterating through a cluster
            for(int i=0; i<INSTRUCTION_CLUSTER_SIZE; i++) {
                // Get valid frame buffer
                while (!xQueueReceive(g_unusedFrameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));

                // Processing Buffer
                gen_processInstruction(new_instruction_buffer, i, new_frame_buffer);

                // Send resulting buffer to PIO
                while(!xQueueSend(g_frameBufferQueue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
            }
            gpio_put(17, false);

            // Returning Instruction buffer
            while(!xQueueSend(g_unusedInstructionBufferQueue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
        }
    }
}

