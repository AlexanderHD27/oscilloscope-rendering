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

extern QueueHandle_t g_unusedInstructionBufferQueue;
extern QueueHandle_t g_instructionBufferQueue;
extern QueueHandle_t g_unusedFrameBufferQueue;
extern QueueHandle_t g_frameBufferQueue;

extern uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
extern uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];