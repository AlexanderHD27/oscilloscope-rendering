/**
 * @file globals.c
 * @author AlexanderHD27
 * @brief Hold global variables like frame/instruction buffers, task Handles and queues
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"

#include "dac.h"

QueueHandle_t unused_instruction_buffer_queue;
QueueHandle_t instruction_buffer_queue;
QueueHandle_t unused_frame_buffer_queue;
QueueHandle_t frame_buffer_queue;

TaskHandle_t processingJobTask;

uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];
