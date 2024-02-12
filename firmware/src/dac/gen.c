#include "pico/stdlib.h"
#include "dac.h"

#include "include_globals.h"

// Signal Generator Stuff

/**
 * handles the processing/signal generation part. It should run on core1
*/
void processing_job_task_function(void * param) {
    job current_job;
    while (true) {
        // Wait until job is available
        if(xQueueReceive(job_queue, &current_job, TICKS_WAIT_DURATION)) {
            gpio_put(17, true);
            process_job(&current_job);
            gpio_put(17, false);

            // Send resulting buffer to PIO
            while(!xQueueSend(data_buf_queue, &(current_job.buf), TICKS_WAIT_DURATION)) {}
        
            // Send Empty Instruction Buffer to Input
            current_job.buf.buf_size = 0;
            current_job.buf.buffer = NULL;

            while(!xQueueSend(unused_instruction_queue, &(current_job), TICKS_WAIT_DURATION)) {}
        }
    }
}

/**
 * Process on job
 * TODO: Implement signal generation function
*/
void process_job(job * j) {
    sleep_ms(1);  
}
