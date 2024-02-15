#ifndef GEN_COMMON_INCLUDE
#define GEN_COMMON_INCLUDE

#define PI 3.14159265359

// Frame Buffer defs
typedef uint16_t (* buffer_pointer_t)[];

typedef struct {
    buffer_pointer_t buffer;
    size_t size;
} frame_buffer_t;

// Instruction Buffer defs
enum InstructionID {
    NONE      = 0x0,
    CONST     = 0x1,
    LINE      = 0x2,
    CUBIC     = 0x3,
    QUADRATIC = 0x4,
    SINE      = 0x5,
};

enum InstructionChannelTarget {
    X = 0x00, 
    Y = 0x80,
};

typedef struct  {
    enum InstructionID id;
    uint16_t length;
    uint16_t param[5];
} instruction_t;

typedef struct {
    uint8_t * buffer;
    size_t size;
} instructionBuffer_t;

#include "dac.h"

// Management/Handler functions
void gen_processInstruction(instructionBuffer_t instructions, frame_buffer_t target_frame_buffer);
void gen_executeInstruction(instruction_t ins, uint16_t * buffer);

// Generator function
void gen_generateLine(instruction_t ins, uint16_t * buffer);
void gen_generateQuadraticFloat(instruction_t ins, uint16_t * buffer);
void gen_generateQuadratic(instruction_t ins, uint16_t * buffer);
void gen_generateCubic(instruction_t ins, uint16_t * buffer);
void gen_generateSineBhaskra(instruction_t ins, uint16_t * buffer);
void gen_generateSineTaylor(instruction_t ins, uint16_t * buffer);

// Util function for pre-generating some common wave forms
void gen_pregenCalibrationCross(__uint16_t * buffer, size_t buffer_size);
void gen_pregenSine(__uint16_t * buffer, size_t buffer_size, float freq_x, float freq_y, float phase_x, float phase_y, float amp_x, float y_amp);
void gen_pregenChirp(__uint16_t * buffer, size_t buffer_size);
void gen_pregenRect(__uint16_t * buffer, size_t buffer_size, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void gen_pregenSquareWave(__uint16_t * buffer, size_t buffer_size);

// Util function for adding instruction to instruction list
size_t gen_addInstructionNone(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length);
size_t gen_addInstructionConst(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t level);
size_t gen_addInstructionLine(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t to);
size_t gen_addInstructionQuadratic(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t to, uint16_t ctrl);
size_t gen_addInstructionCubic(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t ctrl_from, uint16_t ctrl_to, uint16_t to);
size_t gen_addInstructionSine(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, uint16_t from, uint16_t to, uint16_t period, uint16_t phase);

#endif

