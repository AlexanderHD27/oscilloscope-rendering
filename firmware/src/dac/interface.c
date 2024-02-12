#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "dac.h"
#include "include_globals.h"

// from globals.c
extern TaskHandle_t processing_job_task;

void submit_instructions(void * instructions_list, size_t instructions_list_size) {
    instruction_buffer_t instruction_buffer;
    instruction_buffer.buffer = instructions_list;
    instruction_buffer.size = instructions_list_size;
    
    while(!xQueueSend(instruction_buffer_queue, &instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION));
};


void * acquire_instruction_buffer_pointer() {
    instruction_buffer_t instruction_buffer;
    while(!xQueueReceive(unused_instruction_buffer_queue, &instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    return instruction_buffer.buffer;
};

/**
 * Task, that handles filling all queues should run with high priority
*/
TaskHandle_t init_fill_queue_task;

void init_fill_queues_task_function(void * param) {
    for(int i=0; i<GENERAL_QUEUE_SIZE; i++) {
        frame_buffer_t new_frame_buffer;
        new_frame_buffer.size = BUFFER_SIZE;
        new_frame_buffer.buffer = &main_frame_buffers[i];
        while(!xQueueSend(frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    }

    for(int i=0; i<GENERAL_QUEUE_SIZE-2; i++) {
        instruction_buffer_t new_instruction_buffer;
        new_instruction_buffer.size = 0;
        new_instruction_buffer.buffer = &main_instruction_buffer[i];
        while(!xQueueSend(unused_instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    }

    vTaskDelete(NULL);    
}

void __init_tasks() {
    xTaskCreate(processing_job_task_function, "Signal Process Task", 512, NULL, 1, &processing_job_task);
}

void init_dac(PIO pio, uint sm, uint data_pin_start, uint control_pin_start) {
    unused_instruction_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(instruction_buffer_t));
    instruction_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(instruction_buffer_t));
    unused_frame_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(frame_buffer_t));
    frame_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(frame_buffer_t));

    xTaskCreate(init_fill_queues_task_function, "Init fill Queues", 64, NULL, 4, &init_fill_queue_task);

    for(int i=0; i<GENERAL_QUEUE_SIZE; i++)
        pregen_calibration_cross(main_frame_buffers[i], BUFFER_SIZE);

    __init_tasks();
    __init_dac_driver(pio, sm, data_pin_start, control_pin_start);
}