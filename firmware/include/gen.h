#include "dacConfig.h"

/**
 * @file gen.h
 * @author AlexanderHD27
 * @brief Header of code regarding to signal generation
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef GEN_COMMON_INCLUDE
#define GEN_COMMON_INCLUDE

/**
 * @brief Definition of ln(-1)/i
 */
#define PI 3.14159265359

/**
 * @brief pointer to a frame buffer array (pointer -> pointer -> first element of frame buffer array)
 * 
 * @return typedef uint16_t(*)[] 
 */
typedef uint16_t (* buffer_pointer_t)[];

/**
 * @brief hold size and buffer pointer of a frame buffer
 */
/**
 * @var frameBuffer_t::size
 * This is not the actual size, but how many samples it holds per Channel.
 * This means the actual size is size * 2 * sizeof(uint16_t)
*/
/**
 * @var frameBuffer_t::size
 * Pointer to the preallocated buffer of the size of @ref frameBuffer_t::size * 2 * sizeof(uint16_t)
*/
typedef struct {
    buffer_pointer_t buffer;
    size_t size;
} frameBuffer_t;

/**
 * @brief IDs of instruction for signal generation
 */
enum InstructionID {
    NONE      = 0x0,
    CONST     = 0x1,
    LINE      = 0x2,
    CUBIC     = 0x3,
    QUADRATIC = 0x4,
    SINE      = 0x5,
};

/**
 * @brief enum for selecting the channel, a instruction should be written to
 * By ORing this with the @ref InstructionID you get the correct instruction
 */
enum InstructionChannelTarget {
    X = 0x00, 
    Y = 0x80,
};

/**
 * @brief Holds data of one instruction
 */
/**
 * @var instruction_t::length
 * how many samples should be generated must be (in total with all the other instruction for each channel) should be equal to @ref BUFFER_SIZE. 
 * For processing with @ref gen_executeInstruction the @ref frameBuffer_t is not overwritten, so old data remains in it
*/
/**
 * @var instruction_t::id
 * What instruction to execute. The instruction ID is ORed with the @ref InstructionChannelTarget to select the correct channel to write to
*/
/**
 * @var instruction_t::param
 * parameter list to the instruction. Note: Not all parameter are always used
*/
typedef struct  {
    enum InstructionID id;
    uint16_t length;
    uint16_t param[5];
} instruction_t;

/**
 * @brief hold size and buffer pointer of a instruction buffer
 */
/**
 * @var instructionBufferClusters_t::size
 * The actual size in bytes that are instruction (not the number of instructions)
*/
/**
 * @var instructionBufferClusters_t::buffer
 * The pointer to the actual preallocated-buffer
*/
typedef struct {
    uint8_t * buffer;
    size_t * size;
} instructionBufferClusters_t;

#include "dac.h"

// Management/Handler functions

/**
 * @brief Process a instruction list by parsing and executing every instruction
 * This is called by the @ref gen_processingTaskFunction
 * 
 * @param cluster_num Selects the cluster from the instruction buffer, that should be processed
 * @param instructions Holds size and pointer to the instruction to process
 * @param targetFrameBuffer Target FrameBuffer that is filled with resulting signal. The Buffer is not overwrite, so hold data may remain in it 
 */
void gen_processInstruction(instructionBufferClusters_t instructions, uint cluster_num, frameBuffer_t target_frame_buffer);

/**
 * @brief Executes a single already parsed instruction and writes the result to the buffer
 * This called by the @ref gen_processInstruction
 * 
 * @param ins parsed instruction
 * @param buffer target buffer. The resulting data is written from the start of the buffer with a spacing of 2 (only every second element is filled) 
 */
void gen_executeInstruction(instruction_t ins, uint16_t * buffer);

// Generator function

/** 
 * @brief Generates a line by instruction
 * 
 * @param ins parsed instruction with the @ref InstructionID of LINE
 * @param buffer target buffer. The resulting data is written from the start of the buffer with a spacing of 2 (only every second element is filled) 
 */
void gen_generateLine(instruction_t ins, uint16_t * buffer);

/** 
 * @deprecated
 * @brief Generates a quadratic Bézier Curve by instruction.
 * 
 * see <a href="https://www.desmos.com/calculator/aji7g2mrek">desmos</a>
 * 
 * This implementation with floats is to slow -> use of fixpoint implementation: @ref gen_generateQuadratic
 * 
 * This takes ~79,2 ms for 4096 Samples (2 channels)
 * 
 * @param ins parsed instruction with the @ref InstructionID of QUADRATIC
 * @param buffer target buffer. The resulting data is written from the start of the buffer with a spacing of 2 (only every second element is filled) 
 */
void gen_generateQuadraticFloat(instruction_t ins, uint16_t * buffer);

/** 
 * @brief Generates a quadratic Bézier Curve by instruction.
 * 
 * see <a href="https://www.desmos.com/calculator/aji7g2mrek">desmos</a>
 * 
 * This implementation fixpoint arithmetic
 * 
 * This takes ~17.0ms to compute for 4096 Samples (2 Channels)
 * 
 * @param ins parsed instruction with the @ref InstructionID of QUADRATIC
 * @param buffer target buffer. The resulting data is written from the start of the buffer with a spacing of 2 (only every second element is filled) 
 */
void gen_generateQuadratic(instruction_t ins, uint16_t * buffer);

/**
 * @brief Generates a cubic Bézier Curve by instruction.
 * 
 * see <a href="https://www.desmos.com/calculator/usunemqqbi">desmos</a>
 * 
 * This implementation fixpoint arithmetic
 * 
 * This takes ~17.0ms to compute for 4096 Samples (2 Channels)
 * 
 * @param ins parsed instruction with the @ref InstructionID of CUBIC
 * @param buffer target buffer. The resulting data is written from the start of the buffer with a spacing of 2 (only every second element is filled) 
 */
void gen_generateCubic(instruction_t ins, uint16_t * buffer);

/**
 * @deprecated
 * @brief Generates a sine wave by instruction
 * 
 * see <a href="see https://www.desmos.com/calculator/3tu2pwap4c">desmos</a>
 * 
 *  This uses Bhaskra I's Approximation with fixpoint arithmetic. 
 *  This is bad and to slow
 * 
 * @param ins parsed instruction with the @ref InstructionID of SINE
 * @param buffer target buffer. The resulting data is written from the start of the buffer with a spacing of 2 (only every second element is filled)
 */
void gen_generateSineBhaskra(instruction_t ins, uint16_t * buffer);

/**
 * @brief Generates a sine wave by instruction
 * 
 * see <a href="https://www.desmos.com/calculator/77trwqesdm">desmos</a>
 * 
 * This uses taylor-series to approx. a sin wave. Its reasonable accurate, but takes long to computer
 * 
 * This takes ~65.9ms to compute for 4096 Samples (2 Channels) (This is too slow) 
 * TODO: Optimize this more. We nee to get under 30ms 
 * 
 * @param ins 
 * @param buffer 
 */
void gen_generateSineTaylor(instruction_t ins, uint16_t * buffer);

// Util function for pre-generating some common wave forms

/**
 * Pregenerate a circle + cross + point for calibration/testing. This should not be used for live-generation
 * 
 * @param buffer target buffer, that the signal is written to (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size how big is your buffer
*/
void gen_pregenCalibrationCross(__uint16_t * buffer, size_t buffer_size);

/**
 * @brief Generates two sin-waves with corrsponding parameters
 * 
 * @param buffer target buffer, that the signal is written to (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size how big is your buffer
 * @param freq_x frequency of the x-signal in 1/buffer_period
 * @param freq_y frequency of the y-signal in 1/buffer_period
 * @param phase_x phase-shift of the x-signal
 * @param phase_y phase-shift of the y-signal
 * @param amp_x amplitude of x-signal (1 = max signal strength)
 * @param amp_y amplitude of y-signal (1 = max signal strength)
*/
void gen_pregenSine(__uint16_t * buffer, size_t buffer_size, float freq_x, float freq_y, float phase_x, float phase_y, float amp_x, float y_amp);

/**
 * @brief Generates two chirps from -1 to +1 with a period of the buffer size
 * 
 * @param buffer target buffer, that the signal is written to (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size sample size of your buffer
*/
void gen_pregenChirp(__uint16_t * buffer, size_t buffer_size);

/**
 * @brief Generates a rectangle from (x0, y0) to (x1, y1). Uses the whole buff_size for generation
 * @param buffer the target, that the signal is written two (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size sample size of your buffer
*/
void gen_pregenRect(__uint16_t * buffer, size_t buffer_size, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Generates a two square at maximum frequency (the y channel is inverted)
 * 
 * @param buffer the target, that the signal is written two (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size sample size of your buffer
*/
void gen_pregenSquareWave(__uint16_t * buffer, size_t buffer_size);

// Util function for adding instruction to instruction list

/**
 * @brief Adds instruction to generates 0x7fff (= 0) for length samples
 * 
 * @param buffer Target buffer were the instruction is written to
 * @param channel Either X or Y Channel
 * @param length How many samples should be generated
 * @return size_t many bytes were written to the instruction buffer
 */
size_t gen_addInstructionNone(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length);

/**
 * @brief Adds instruction to generates level for length samples
 * 
 * @param buffer Target buffer were the instruction is written to
 * @param channel Either X or Y Channel
 * @param length How many samples should be generated
 * @param level is between 0x0000 and 0xffff (-1 and +1)
 * @return size_t many bytes were written to the instruction buffer
 */
size_t gen_addInstructionConst(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t level);

/**
 * @brief Adds instruction to lerp from "from" to "to"
 * 
 * @param buffer Target buffer were the instruction is written to
 * @param channel Either X or Y Channel
 * @param length How many samples should be generated
 * @param from where to start. Is between 0x0000 and 0xffff (-1 and +1)
 * @param to where to stop. Is between 0x0000 and 0xffff (-1 and +1)
 * @return size_t many bytes were written to the instruction buffer
 */
size_t gen_addInstructionLine(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t to);

/**
 * @brief Adds instruction to generate a cubic Bézier curve from "from" to "to" via the control point ctrl
 * 
 * @param buffer Target buffer were the instruction is written to
 * @param channel Either X or Y Channel
 * @param length How many samples should be generated
 * @param from where to start. Is between 0x0000 and 0xffff (-1 and +1)
 * @param to where to stop. Is between 0x0000 and 0xffff (-1 and +1)
 * @param ctrl Bézier control point. Is between 0x0000 and 0xffff (-1 and +1)
 * @return size_t many bytes were written to the instruction buffer
 */
size_t gen_addInstructionQuadratic(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t to, uint16_t ctrl);

/**
 * @brief Adds instruction to generate a quadratic Bézier curve from "from" to "to" via the control point ctrl_from and ctrl_to
 * 
 * @param buffer Target buffer were the instruction is written to
 * @param channel Either X or Y Channel
 * @param length How many samples should be generated
 * @param from where to start. Is between 0x0000 and 0xffff (-1 and +1)
 * @param to where to stop. Is between 0x0000 and 0xffff (-1 and +1)
 * @param ctrl_from point that controls the behavior near from-point. Is between 0x0000 and 0xffff (-1 and +1)
 * @param ctrl_to point that controls the behavior near to-point. Is between 0x0000 and 0xffff (-1 and +1)
 * @return size_t many bytes were written to the instruction buffer
 */
size_t gen_addInstructionCubic(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t ctrl_from, uint16_t ctrl_to, uint16_t to);

/**
 * @brief Adds instruction to generate a sine according to the parameters from, to, period and phase
 * 
 * @param buffer Target buffer were the instruction is written to
 * @param channel Either X or Y Channel
 * @param length How many samples should be generated
 * @param from the low point of the sine wave. Is between 0x0000 and 0xffff (-1 and +1)
 * @param to the high point of the sine wave. Is between 0x0000 and 0xffff (-1 and +1)
 * @param period How many samples a cycle should take. This should be <= @ref BUFFER_SIZE for a full sine wave
 * @param phase phase-shift in samples. This should be <= @ref BUFFER_SIZE
 * @return size_t many bytes were written to the instruction buffer
 */
size_t gen_addInstructionSine(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t to, uint16_t period, uint16_t phase);

#endif

