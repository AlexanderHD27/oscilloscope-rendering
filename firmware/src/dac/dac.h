#ifndef DAC_CUSTOM
#define DAC_CUSTOM

#include "pico/types.h"
#include "hardware/pio.h"

#include "dacConfig.h"

#define CONV_NORMAL_TO_UINT16(x) (uint16_t)( 0xffff*((x+1)/2) )
#define PI 3.14159265359

// Data Types for Buffer-flow
typedef uint16_t (* buffer_pointer_t)[];

typedef struct {
    buffer_pointer_t buffer;
    size_t size;
} frame_buffer_t;

typedef struct {
    uint8_t * buffer;
    size_t size;
} instruction_buffer_t;

// Main Init function
void init_dac(PIO pio, uint sm, uint data_pin_start, uint control_pin_start);
void __init_dac_driver(PIO pio, uint sm, uint data_pin_start, uint control_pin_start);

// Tasks
void in_task_function(void * param);
void processing_job_task_function(void * param);

void process_job(instruction_buffer_t instructions, frame_buffer_t target_frame_buffer);

// Instruction

enum INSTRUCTION_ID {
    NONE      = 0x0,
    CONST     = 0x1,
    LINE      = 0x2,
    CUBIC     = 0x3,
    QUADRATIC = 0x4,
    SINE      = 0x5,
};

enum INSTRUCTION_SEL_CHANNEL {
    X = 0x00, 
    Y = 0x80,
};

typedef struct  {
    enum INSTRUCTION_ID id;
    uint16_t length;
    uint16_t param[5];
} __instruction_t;

void execute_instruction(__instruction_t ins, uint16_t * buffer);

size_t add_ins_none(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length);
size_t add_ins_const(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length, uint16_t level);
size_t add_ins_line(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length, uint16_t from, uint16_t to);

/**
 * Submits a new instruction list to the dac. This function should be use by the IO interface (e.g. USB) to submit an instruction list
 * @param instructions_list pointer to the instruction array. Should be pointer acquired from get_instruction_buf()
 * @param instructions_list_size size of the instruction list (how many bytes are used for instruction, not size of the instruction_list -> instruction_list_size < sizeof(instruction_list))
*/
void submit_instructions(uint8_t * instructions_list, size_t instructions_list_size);

/**
 * acquires a buffer for instructions to be written to. The size of the instruction list terminated by INSTRUCTION_BUF_SIZE in bytes 
*/
uint8_t * acquire_instruction_buffer_pointer();

// Util function for pre-generating some common wave forms
void pregen_calibration_cross(__uint16_t * buffer, size_t buffer_size);
void gen_sins(__uint16_t * buffer, size_t buffer_size, float freq_x, float freq_y, float phase_x, float phase_y, float amp_x, float y_amp);
void pregen_chirp(__uint16_t * buffer, size_t buffer_size);
void pregen_rect(__uint16_t * buffer, size_t buffer_size, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void pregen_square_wave(__uint16_t * buffer, size_t buffer_size);

#endif