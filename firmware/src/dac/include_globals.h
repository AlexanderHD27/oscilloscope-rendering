#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "dac.h"

extern QueueHandle_t unused_instruction_queue;
extern QueueHandle_t instruction_queue;
extern QueueHandle_t job_queue;
extern QueueHandle_t data_buf_queue;
extern QueueHandle_t unused_data_buf_queue;

extern uint16_t main_frame_buffers[GENERAL_QUEUE_SIZE][BUFFER_SIZE*2];
extern uint8_t  main_instruction_buffer[GENERAL_QUEUE_SIZE][INSTRUCTION_BUF_SIZE];