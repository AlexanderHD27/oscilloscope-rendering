#include "pico/stdlib.h"
#include "dac.h"

#include "include_globals.h"

// Signal Generator Stuff

/**
 * handles the processing/signal generation part. It should run on core1
*/
void processing_job_task_function(void * param) {
    instruction_buffer_t new_instruction_buffer;
    frame_buffer_t new_frame_buffer;

    while (true) {
        // Wait until instruction are available
        if(xQueueReceive(instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {

            // Get valid frame buffer
            while (!xQueueReceive(unused_frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
            

            gpio_put(17, true);
            process_job(new_instruction_buffer, new_frame_buffer);
            gpio_put(17, false);

            // Send resulting buffer to PIO
            while(!xQueueSend(frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}

            // Returning Instruction buffer
            while(!xQueueSend(unused_instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
        }
    }
}

/**
 * Process on job
 * TODO: Implement signal generation function
*/
void process_job(instruction_buffer_t instructions, frame_buffer_t target_frame_buffer) {
    pregen_chirp(target_frame_buffer.buffer, target_frame_buffer.size);
}
