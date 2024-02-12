#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "dac.h"
#include "include_globals.h"

// from globals.c
extern TaskHandle_t in_task;
extern TaskHandle_t processing_job_task;

void in_task_function(void * param) {
    job empty_job;
    frame_data_buffer unused_data_buf;    
    
    while (true) {
        while(!xQueueReceive(instruction_queue, &empty_job, 1)) {}
        while(!xQueueReceive(unused_data_buf_queue, &unused_data_buf, 1)) {}

        while (!xQueueSend(job_queue, &empty_job, 1)) {}
    }
}

void submit_instructions(void * instructions_list, size_t instructions_list_size) {
    job new_job;
    new_job.ins_buf = instructions_list;
    new_job.ins_buf_size = instructions_list_size;

    while(!xQueueSend(instruction_queue, &new_job, 1)) {};
};


void * acquire_instruction_buf() {
    job old_job;    
    while(!xQueueReceive(instruction_queue, &old_job, 1)) {};
    return old_job.ins_buf;
};

/**
 * Task, that handles filling all queues should run with high priority
*/
TaskHandle_t init_fill_queue_task;

void init_fill_queues_task_function(void * param) {
    for(int i=0; i<2; i++) {
        frame_data_buffer new_buf;
        new_buf.buf_size = BUFFER_SIZE;
        new_buf.buffer = &main_frame_buffers[i];
        while(!xQueueSend(data_buf_queue, &new_buf, 1));
    }

    for(int i=0; i<GENERAL_QUEUE_SIZE-2; i++) {
        frame_data_buffer new_buf;
        new_buf.buf_size = BUFFER_SIZE;
        new_buf.buffer = &main_frame_buffers[i+2];
        while(!xQueueSend(unused_data_buf_queue, &new_buf, 1));
    }

    for(int i=0; i<GENERAL_QUEUE_SIZE; i++) {
        job new_job;
        new_job.ins_buf = &main_instruction_buffer[i];
        new_job.ins_buf_size = INSTRUCTION_BUF_SIZE;
        while(!xQueueSend(unused_instruction_queue, &new_job, 1));
    }

    vTaskDelete(NULL);    
}

void __init_tasks() {
    xTaskCreate(in_task_function, "In Task", 512, NULL, 1, &in_task);
    xTaskCreate(processing_job_task_function, "Signal Process Task", 512, NULL, 1, &processing_job_task);
}

void init_dac(PIO pio, uint sm, uint data_pin_start, uint control_pin_start) {
    unused_instruction_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(job));
    instruction_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(job));
    job_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(job));
    data_buf_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(frame_data_buffer));
    unused_data_buf_queue = xQueueCreate(GENERAL_QUEUE_SIZE+2, sizeof(frame_data_buffer));

    xTaskCreate(init_fill_queues_task_function, "Init fill Queues", 64, NULL, 4, &init_fill_queue_task);

    for(int i=0; i<GENERAL_QUEUE_SIZE; i++)
        pregen_calibration_cross(main_frame_buffers[i], BUFFER_SIZE);

    __init_tasks();
    __init_dac_driver(pio, sm, data_pin_start, control_pin_start);
}