#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"

#include "dac.h"

QueueHandle_t unused_instruction_queue;
QueueHandle_t instruction_queue;
QueueHandle_t job_queue;
QueueHandle_t data_buf_queue;
QueueHandle_t unused_data_buf_queue;

TaskHandle_t in_task;
TaskHandle_t processing_job_task;

uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];
