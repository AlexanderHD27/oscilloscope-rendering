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

QueueHandle_t g_unusedInstructionBufferQueue;
QueueHandle_t g_instructionBufferQueue;
QueueHandle_t g_unusedFrameBufferQueue;
QueueHandle_t g_frameBufferQueue;

TaskHandle_t processingJobTask;

uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];
