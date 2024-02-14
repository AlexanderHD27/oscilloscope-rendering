#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"

#include "dac.h"

QueueHandle_t unused_instruction_buffer_queue;
QueueHandle_t instruction_buffer_queue;
QueueHandle_t unused_frame_buffer_queue;
QueueHandle_t frame_buffer_queue;

TaskHandle_t processing_job_task;

uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];
