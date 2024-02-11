#include <stdio.h>
#include "pico/stdlib.h"

#include "gen.h"

/**
 * Main Loop for core0. Does the signal/job processing
*/
void gen_mainloop_entry1() {
    if(multicore_fifo_pop_blocking() != FLAG_VALUE) {
        // If this is run on the wrong core, we just lock it it out
        tight_loop_contents();
    }
    generation_job * job;

    while (true) {
        job = multicore_fifo_pop_blocking();
        process_generation_job(job);
        multicore_fifo_push_blocking(job);
    }
}

/**
 * Process one job and writes the resulting signal into the job buffer
*/
void process_generation_job(generation_job * job) {
    gpio_put(17, true);
    sleep_ms(1);
    gpio_put(17, false);
}