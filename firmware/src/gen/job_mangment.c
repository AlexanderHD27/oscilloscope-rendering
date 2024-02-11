#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "gen.h"
#include "dac.h"

#define JOB_QUEUE_SIZE 4
#define BUFFER_SIZE 0x1000

// Main Frame Buffers
uint16_t frame_buffers [JOB_QUEUE_SIZE + 2][BUFFER_SIZE*2];
bool frame_buffers_used [JOB_QUEUE_SIZE];

// Job Queue
generation_job job_queue[JOB_QUEUE_SIZE];
size_t job_queue_font = 0;
size_t job_queue_rear = -1;

uint16_t (* output_buffer_a_ptr)[];
size_t output_buffer_a_ptr_index = 0;
uint16_t (* output_buffer_b_ptr)[];
size_t output_buffer_b_ptr_index = 0;

void handle_next_frame(BUF_SEL empty_buf) {
    generation_job * job = recv_finished_job();

    if(job != NULL) {
        // New Frame -> Swap out now empty_buffer
        if(empty_buf == A) {
            frame_buffers_used[output_buffer_a_ptr_index] = false;
        } else if(empty_buf == B) {
            frame_buffers_used[output_buffer_b_ptr_index] = false;
        }
    } else {
        // No new Frame available, so we just repeat the current one   
    }
}

int enqueue_new_job(instructions ins) {
    int res = 0;
    if(job_queue_font == job_queue_rear) {
        // Job queue is full, so we just drop the job
        res = -1;
    } else {

    }

    return res;
}

/**
 * Sends a job to the processing core, if fifo has spaces. Should be run on core0
 * @param job pointer to the job that should be processed. Should only be overwritten if returned via the other fifo
 * @return If fifo is full -1 else 0 
*/
int send_job_to_processing(generation_job * job) {
    if(multicore_fifo_wready()) {
        multicore_fifo_push_blocking((uint32_t)job);
        return 0;
    } else {
        return -1;
    }
}

/**
 * Pulls a job (if available) from the processing core. Should be run on core0
 * @return returns pointer to the finished job, or NULL (if non are finished)
*/
generation_job * recv_finished_job() {
    if(multicore_fifo_rvalid()) {
        generation_job * job = multicore_fifo_pop_blocking();
    } else {
        return NULL;
    }
}

void gen_init() {
    multicore_reset_core1();
    multicore_launch_core1(gen_mainloop_entry1);
    multicore_fifo_push_blocking(FLAG_VALUE);
}