/**
 * @file interface.c
 * @author AlexanderHD27
 * @brief Contains Code regarding Setup and Initialisation
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "dac.h"
#include "gen.h"
#include "includeGlobals.h"

// from globals.c
extern TaskHandle_t processingJobTask;

void dac_submitInstructions(uint8_t * instructions_list, size_t instructions_list_size) {
    instructionBuffer_t instruction_buffer;
    instruction_buffer.buffer = instructions_list;
    instruction_buffer.size = instructions_list_size;
    
    while(!xQueueSend(instruction_buffer_queue, &instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION));
};


uint8_t * dac_acquireInstructionBufferPointer() {
    instructionBuffer_t instruction_buffer;
    while(!xQueueReceive(unused_instruction_buffer_queue, &instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    return instruction_buffer.buffer;
};

/**
 * Task, that handles filling all queues should run with high priority
*/
TaskHandle_t init_fill_queue_task;

/**
 * @brief Task function for init fill of queues
 * see DataFlow Diagram
 * 
 * @param param Not used, can be ignored
 */
static void initFillQueuesTaskFunction(void * param) {
    for(int i=0; i<GENERAL_QUEUE_SIZE; i++) {
        frame_buffer_t new_frame_buffer;
        new_frame_buffer.size = BUFFER_SIZE;
        new_frame_buffer.buffer = &main_frame_buffers[i];
        while(!xQueueSend(frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    }

    for(int i=0; i<GENERAL_QUEUE_SIZE-2; i++) {
        instructionBuffer_t new_instruction_buffer;
        new_instruction_buffer.size = 0;
        new_instruction_buffer.buffer = (uint8_t *)(&main_instruction_buffer[i]);
        while(!xQueueSend(unused_instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION));
    }

    vTaskDelete(NULL);    
}

/**
 * @brief Inits and start all relevant task
 * Internally used
 */
static void initTasks() {
    xTaskCreate(gen_processingTaskFunction, "Signal Process Task", 1024, NULL, 1, &processingJobTask);
    xTaskCreate(initFillQueuesTaskFunction, "Init fill Queues", 64, NULL, 4, &init_fill_queue_task);
}

void dac_init(PIO pio, uint sm, uint data_pin_start, uint control_pin_start) {
    unused_instruction_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(instructionBuffer_t));
    instruction_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(instructionBuffer_t));
    unused_frame_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(frame_buffer_t));
    frame_buffer_queue = xQueueCreate(GENERAL_QUEUE_SIZE, sizeof(frame_buffer_t));


    for(int i=0; i<GENERAL_QUEUE_SIZE; i++)
        gen_pregenCalibrationCross(main_frame_buffers[i], BUFFER_SIZE);

    initTasks();
    _dac_initPIO(pio, sm, data_pin_start, control_pin_start);
}