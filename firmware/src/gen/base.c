/**
 * @file base.c
 * @author AlexanderHd27
 * @brief Contains Code that parses a Instruction (List)
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "pico/stdlib.h"
#include "dac.h"
#include "gen.h"

#include "includeGlobals.h"

// Signal Generator Stuff
enum PARSER_STATE {
    READ_ID,
    READ_LENGTH_HIGH,
    READ_LENGTH_LOW,
    READ_PARAM_HIGH,
    READ_PARAM_LOW,
};

void gen_processInstruction(instructionBuffer_t instructions, frameBuffer_t target_frame_buffer) {
    enum PARSER_STATE parse_state = READ_ID;
    uint param_count = 0;
    uint param_index = 0;
   
    uint8_t current_byte;
    instruction_t ins;
    enum InstructionChannelTarget channel;

    size_t buffer_offset_x = 0;
    size_t buffer_offset_y = 0;

    // This implements a state machine for parsing the instruction list
    for(int i=0; i<instructions.size; i++) { 
        current_byte = instructions.buffer[i];

        switch (parse_state) {
        // Reading Instruction ID 
        case READ_ID: {
            ins.id = current_byte & 0x7f;
            parse_state = READ_LENGTH_HIGH;
            param_index = 0;
            channel = current_byte & 0x80;

            switch (ins.id) {
            case NONE:
                param_count = 0;
                break;
            case CONST:
                param_count = 1;
                break;
            case LINE:
                param_count = 2;
                break;
            case QUADRATIC:
                param_count = 3;
                break;
            case CUBIC:
                param_count = 4;
                break;
            case SINE:
                param_count = 4;
                break;

            default:
                // If its not valid id instruction, just read the next as the id
                parse_state = READ_ID;
                break;
            }
        }   break;

        // Reading Length
        case READ_LENGTH_HIGH: {
            ins.length = current_byte << 8;
            parse_state = READ_LENGTH_LOW;
        }   break;

        case READ_LENGTH_LOW: {
            ins.length |= current_byte;
            parse_state = READ_PARAM_HIGH;
        }   break;

        // Reading Parameters
        case READ_PARAM_HIGH: {
            ins.param[param_index] = current_byte << 8;
            parse_state = READ_PARAM_LOW;
        }   break;

        case READ_PARAM_LOW: {
            ins.param[param_index] |= current_byte;

            param_count--;
            param_index++;
            
            // Check if we need more parameter
            if(param_count == 0) {
                // No, We are done parsing, we can execute our instruction now
                size_t offset = channel == X ? buffer_offset_x : buffer_offset_y + 1;
                gen_executeInstruction(ins, &((uint16_t * )(target_frame_buffer.buffer))[offset]);

                parse_state =  READ_ID;

                if(channel == X)
                    buffer_offset_x += ins.length*2;
                else
                    buffer_offset_y += ins.length*2;
            } else {
                // Yes, continue reading more parameters
                parse_state = READ_PARAM_HIGH;
            }

        }   break;

        }
    }
}

// Using fix-point notation, the rp2040 has no FPU, so float operation take to long :/
// But the rp2040 has 32bit addition/multiplication
// format: [17].[15] (We cannot use [16].[16] because we need negative numbers and this twos-compliment)
#define FIX_POINT_SHIFT 15
#define TOWS_COMPLIMENT_MASK 0x800000000

void gen_executeInstruction(instruction_t ins, uint16_t * buffer) {
    switch (ins.id) {
    case NONE:
        for(int i=0; i<ins.length; i++)
            buffer[i*2] = 0x7fff;
        break;
    
    case CONST:
        for(int i=0; i<ins.length; i++)
            buffer[i*2] = ins.param[0];
        break;

    case LINE:
        gen_generateLine(ins, buffer);
        break;

    case QUADRATIC:
        gen_generateQuadratic(ins, buffer);
        break;

    case CUBIC:
        gen_generateCubic(ins, buffer);
        break;

    case SINE:
        gen_generateSineTaylor(ins, buffer);
        break;

    default:
        break;
    }
}
