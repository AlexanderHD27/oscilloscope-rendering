/**
 * @file includeGlobals.h
 * @author AlexanderHD27
 * @brief Contains includes for queues and pre-allocated buffers for frame and instruction
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "dac.h"

extern QueueHandle_t unused_instruction_buffer_queue;
extern QueueHandle_t instruction_buffer_queue;
extern QueueHandle_t unused_frame_buffer_queue;
extern QueueHandle_t frame_buffer_queue;

extern uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
extern uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];