/**
 * @file util.c
 * @author AlexanderHD27
 * @brief Testing functions for adding instruction to instruction list
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "dac.h"
#include "instructionFormat.h"

size_t gen_addInstructionNone(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length) {
    buffer[0] = NONE | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    return 3;
}

size_t gen_addInstructionConst(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length,
    uint16_t level) {
    buffer[0] = CONST | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3] = (level  & 0xff00) >> 8;
    buffer[4] = (level  & 0x00ff);
    return 5;
}

size_t dac_addInstructionLine(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length,
    uint16_t from, uint16_t to) {
    buffer[0] = LINE | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3 + 2*PARAM_LINE_FROM_VALUE    ] = (from  & 0xff00) >> 8;
    buffer[3 + 2*PARAM_LINE_FROM_VALUE + 1] = (from  & 0x00ff);
    buffer[3 + 2*PARAM_LINE_TO_VALUE      ] = (to    & 0xff00) >> 8;
    buffer[3 + 2*PARAM_LINE_TO_VALUE   + 1] = (to    & 0x00ff);
    return 7;
}

size_t gen_addInstructionQuadratic(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length,
    uint16_t from, uint16_t ctrl, uint16_t to) {
    buffer[0] = QUADRATIC | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3 + 2*PARAM_QUADRATIC_P0    ] = (from    & 0xff00) >> 8;
    buffer[3 + 2*PARAM_QUADRATIC_P0 + 1] = (from    & 0x00ff);
    buffer[3 + 2*PARAM_QUADRATIC_P2    ] = (to      & 0xff00) >> 8;
    buffer[3 + 2*PARAM_QUADRATIC_P2 + 1] = (to      & 0x00ff);
    buffer[3 + 2*PARAM_QUADRATIC_P1    ] = (ctrl    & 0xff00) >> 8;
    buffer[3 + 2*PARAM_QUADRATIC_P1 + 1] = (ctrl    & 0x00ff);
    return 9;
}

size_t gen_addInstructionCubic(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length,
    uint16_t from, uint16_t ctrl_from, uint16_t ctrl_to, uint16_t to) {
    buffer[0] = CUBIC | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3 + 2*PARAM_CUBIC_P0    ] = (from      & 0xff00) >> 8;
    buffer[3 + 2*PARAM_CUBIC_P0 + 1] = (from      & 0x00ff);
    buffer[3 + 2*PARAM_CUBIC_P1    ] = (ctrl_from & 0xff00) >> 8;
    buffer[3 + 2*PARAM_CUBIC_P1 + 1] = (ctrl_from & 0x00ff);
    buffer[3 + 2*PARAM_CUBIC_P2    ] = (ctrl_to   & 0xff00) >> 8;
    buffer[3 + 2*PARAM_CUBIC_P2 + 1] = (ctrl_to   & 0x00ff);
    buffer[3 + 2*PARAM_CUBIC_P3    ] = (to        & 0xff00) >> 8;
    buffer[3 + 2*PARAM_CUBIC_P3 + 1] = (to        & 0x00ff);
    return 11;
}

size_t gen_addInstructionSine(uint8_t * buffer, enum InstructionChannelTarget channel, uint16_t length, 
    uint16_t from, uint16_t to, uint16_t period, uint16_t phase) {
    buffer[0] = SINE | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3 + 2*PARAM_SIN_FROM      ] = (from      & 0xff00) >> 8;
    buffer[3 + 2*PARAM_SIN_FROM   + 1] = (from      & 0x00ff);
    buffer[3 + 2*PARAM_SIN_TO        ] = (to        & 0xff00) >> 8;
    buffer[3 + 2*PARAM_SIN_TO     + 1] = (to        & 0x00ff);
    buffer[3 + 2*PARAM_SIN_PERIOD    ] = (period    & 0xff00) >> 8;
    buffer[3 + 2*PARAM_SIN_PERIOD + 1] = (period    & 0x00ff);
    buffer[3 + 2*PARAM_SIN_PHASE     ] = (phase     & 0xff00) >> 8;
    buffer[3 + 2*PARAM_SIN_PHASE  + 1] = (phase     & 0x00ff);
    return 11;
}

