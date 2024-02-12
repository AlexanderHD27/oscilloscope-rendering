#include "pico/stdlib.h"
#include "dac.h"

#include "include_globals.h"

// Signal Generator Stuff

/**
 * handles the processing/signal generation part. It should run on core1
*/
void processing_job_task_function(void * param) {
    instruction_buffer_t new_instruction_buffer;
    frame_buffer_t new_frame_buffer;

    while (true) {
        // Wait until instruction are available
        if(xQueueReceive(instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {

            // Get valid frame buffer
            while (!xQueueReceive(unused_frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION));
            

            gpio_put(17, true);
            process_job(new_instruction_buffer, new_frame_buffer);
            gpio_put(17, false);

            // Send resulting buffer to PIO
            while(!xQueueSend(frame_buffer_queue, &new_frame_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}

            // Returning Instruction buffer
            while(!xQueueSend(unused_instruction_buffer_queue, &new_instruction_buffer, DEFAULT_QUEUE_WAIT_DURATION)) {}
        }
    }
}

enum PARSER_STATE {
    READ_ID,
    READ_LENGTH_HIGH,
    READ_LENGTH_LOW,
    READ_PARAM_HIGH,
    READ_PARAM_LOW,
};

/**
 * Process on job
 * TODO: Implement signal generation function
*/
void process_job(instruction_buffer_t instructions, frame_buffer_t target_frame_buffer) {
    enum PARSER_STATE parse_state = READ_ID;
    uint param_count = 0;
    uint param_index = 0;
   
    uint8_t current_byte;
    uint16_t param;
    __instruction_t ins;
    enum INSTRUCTION_SEL_CHANNEL channel;

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
            /*case CUBIC:
                param_count = 3;
                break;
            case QUADRATIC:
                param_count = 4;
                break;
            case SINE:
                param_count = 5;
                break;*/

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
            param = current_byte << 8;
            parse_state = READ_PARAM_LOW;
        }   break;

        case READ_PARAM_LOW: {
            ins.param[param_index] = param;
            ins.param[param_index] |= current_byte;

            param_count--;
            param_index++;
            
            // Check if we need more parameter
            if(param_count == 0) {
                // No, We are done parsing, we can execute our instruction now
                size_t offset = channel == X ? buffer_offset_x : buffer_offset_y + 1;
                execute_instruction(ins, &((uint16_t * )(target_frame_buffer.buffer))[offset]);

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

#define PARAM_CONST_LEVEL 0
#define PARAM_LINE_FROM_VALUE 0
#define PARAM_LINE_TO_VALUE 1

// Using fix-point notation, the rp2040 has no FPU, so float operation take to long :/
// But the rp2040 has 32bit addition/multiplication
// format: [17].[15] (We cannot use [16].[16] because we need negative numbers and this twos-compliment)
#define FIX_POINT_SHIFT 15
#define TOWS_COMPLIMENT_MASK 0x800000000

void execute_instruction(__instruction_t ins, uint16_t * buffer) {
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
        gen_line(ins, buffer);
        break;


    default:
        break;
    }
}

// More Complex generation function
void gen_line(__instruction_t ins, uint16_t * buffer) {
    int32_t delta_y = (ins.param[PARAM_LINE_TO_VALUE] - ins.param[PARAM_LINE_FROM_VALUE]);
    volatile uint32_t delta_x = ((uint32_t)(ins.length-1)) << FIX_POINT_SHIFT;
    volatile int32_t step = ((delta_y << FIX_POINT_SHIFT) | (delta_y & TOWS_COMPLIMENT_MASK)) / delta_x;

    for(size_t i=0; i<ins.length; i++) {
        buffer[i*2] = (((uint32_t)(i) << FIX_POINT_SHIFT) * step) >> FIX_POINT_SHIFT;
    }
}


// Util function for adding instructions to a instruction list

size_t add_ins_none(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length) {
    buffer[0] = NONE | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    return 3;
}

size_t add_ins_const(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length,
    uint16_t level) {
    buffer[0] = CONST | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3] = (level  & 0xff00) >> 8;
    buffer[4] = (level  & 0x00ff);
    return 5;
}

size_t add_ins_line(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length,
    uint16_t from, uint16_t to) {
    buffer[0] = LINE | channel;
    buffer[1] = (length & 0xff00) >> 8;
    buffer[2] = (length & 0x00ff);
    buffer[3] = (from  & 0xff00) >> 8;
    buffer[4] = (from  & 0x00ff);
    buffer[5] = (to    & 0xff00) >> 8;
    buffer[6] = (to    & 0x00ff);
    return 7;
}

