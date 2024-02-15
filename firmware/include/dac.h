/**
 * @file dac.h
 * @author AlexanderHD27
 * @brief Header for code regarding managing the Data/Buffer flow 
 * @version 0.1
 * @date 2024-02-15
 * @copyright Copyright (c) 2024
 * 
 * Dataflow Diagram: 
 * 
 * ![The Logo](docs/drawio/DataFlow_RP2040/DataFlow_RP2040.drawio.png)
 */

#ifndef DAC_CUSTOM
#define DAC_CUSTOM

#include "pico/types.h"
#include "hardware/pio.h"

#include "gen.h"
#include "dacConfig.h"
#include "instructionFormat.h"

#define CONV_NORMAL_TO_UINT16(x) (uint16_t)( 0xffff*((x+1)/2) )
#define PIO_CLK_DIV(freq) (SYS_CLK_MHZ*1000000)/(freq)


/**
 * @brief Inits and setups task, queues and PIO
 * 
 * This should be run before adding instruction to the instruction queue. Further more it pregenerates a calibration 
 * cross signal, that is overwritten when submitting the first instruction to the queue
 * 
 * @param pio PIO core used by the dac: PIO0 or PIO1
 * @param sm PIO State Machine used by the dac
 * @param pinDataStart Start GPIO pins used for data-transfer to the D-Latches. In total 8 Pins are used
 * @param pinCtrlStart Start GPIO pin used for control. In total 4 Pins are used
 * 
 * Pin-Mapping:
 * 
 * pinDataStart to pinDataStart + 7: Data 0-7
 * 
 * pinCtrlStart + 0: Enable_Pre_Load
 * 
 * pinCtrlStart + 1: Sel_A
 * 
 * pinCtrlStart + 2: Sel_B
 * 
 * pinCtrlStart + 3: LD_Master
 */
void dac_init(PIO pio, uint sm, uint pinDataStart, uint pinCtrlStart);

/**
 * 
 * @brief Configures and starts PIO State machines to create the signal for the DACs.
 * This driver use one PIO core and one State machine
 *
 * Pin Layout:
 *      Data Pins: 0-7 (Reverse order)
 *      Control Pins:
 *          0 pre-register LE
 *          1 Sel_A (High or Lower 8 bits) Inverted
 *          2 Sel_B (X or Y Channel) Inverted
 *          3 Master LE
 * 
 * 
 * Buffer is layout as { X0, Y0, X1, X1, ... }
 * This means that a buffer needs to have size of 2*buffer_size (buffer_size is referring to the number of samples it has)
 * The buffer is not passed directly, but as pointer to the array (uint16_t (* buffer_a)[]), such that the buffer can be swap
 * for another without coping data around. Or put this: the pointer to the pointer to the data is a passed
 * The Buffer pointers are store in a global variable, so make sure that these pointers do not go out of scope
 * 
 * @param pio either pio0 or pio1
 * @param sm pio state machine used
 * @param control_pin_start starting offset for the control pins (in total 4 control bins are used)
 * @param data_pin_start starting offset for data connection (in total 8 data pins are used)
 * @param buffer_a pointer to output buffer A (the buffer needs to be of size 2*buffer_size)
 * @param buffer_b pointer to output buffer B (the buffer needs to be of size 2*buffer_size)
 * @param buffer_size who many X-Y points are contained in a frame
 * @param next_frame_handler Handler function, called after a frame swap occurred. Its parsed the used buffer, to modify/fill. This function should talk only short amount of time to execute
*/
void _dac_initPIO(PIO pio, uint sm, uint pinDataStart, uint pinCtrlStart);

// Util Function for adding instruction to the instruction list

/**
 * @brief Task Function, that handles the execution and processing of a instruction list
 * 
 * This should only be called as a task by FreeRTOS. This should run on Core1 at high priority
 * 
 * @param param needs to be here to make FreeRTOS happy, not used
 */
void gen_processingTaskFunction(void * param);

/**
 * Submits a new instruction list to the dac. This function should be use by the IO interface (e.g. USB) to submit an instruction list
 * @param instructions_list pointer to the instruction array. Should be pointer acquired from @ref get_instruction_buf
 * @param instructions_list_size size of the instruction list (how many bytes are used for instruction, not size of the instruction_list -> instruction_list_size < sizeof(instruction_list))
*/
void dac_submitInstructions(uint8_t * instructions_list, size_t instructions_list_size);

/**
 * @brief acquires a buffer for instructions to be written to
 * 
 * After running this, the buffer should be returned via @ref dac_submitInstructions
 * 
 * @return pointer to the uint8_t buffer that should be fill with instructions. The size of the instruction list terminated by @ref INSTRUCTION_BUF_SIZE in bytes 
*/
uint8_t * dac_acquireInstructionBufferPointer();

#endif

