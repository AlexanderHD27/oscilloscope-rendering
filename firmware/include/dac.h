#ifndef DAC_CUSTOM
#define DAC_CUSTOM

#include "pico/types.h"
#include "hardware/pio.h"

#include "gen.h"
#include "dacConfig.h"
#include "instructionFormat.h"

#define CONV_NORMAL_TO_UINT16(x) (uint16_t)( 0xffff*((x+1)/2) )
#define PIO_CLK_DIV(freq) (SYS_CLK_MHZ*1000000)/(freq)


// Main Init function
void dac_init(PIO pio, uint sm, uint data_pin_start, uint control_pin_start);
void _dac_initPIO(PIO pio, uint sm, uint data_pin_start, uint control_pin_start);

// Util Function for adding instruction to the instruction list

// This task handles the actual execution/processing of a instruction list
void gen_processingTaskFunction(void * param);

/**
 * Submits a new instruction list to the dac. This function should be use by the IO interface (e.g. USB) to submit an instruction list
 * @param instructions_list pointer to the instruction array. Should be pointer acquired from get_instruction_buf()
 * @param instructions_list_size size of the instruction list (how many bytes are used for instruction, not size of the instruction_list -> instruction_list_size < sizeof(instruction_list))
*/
void dac_submitInstructions(uint8_t * instructions_list, size_t instructions_list_size);

/**
 * acquires a buffer for instructions to be written to. The size of the instruction list terminated by INSTRUCTION_BUF_SIZE in bytes 
*/
uint8_t * dac_acquireInstructionBufferPointer();

#endif

