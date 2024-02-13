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
            case QUADRATIC:
                param_count = 3;
                break;
            case CUBIC:
                param_count = 4;
                break;
            /*case SINE:
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

#define PARAM_QUADRATIC_P0 0
#define PARAM_QUADRATIC_P1 1
#define PARAM_QUADRATIC_P2 2

#define PARAM_CUBIC_P0 0
#define PARAM_CUBIC_P1 1
#define PARAM_CUBIC_P2 2
#define PARAM_CUBIC_P3 3

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
        __gen_line(ins, buffer);
        break;

    case QUADRATIC:
        __gen_quadratic(ins, buffer);
        break;

    case CUBIC:
        __gen_cubic(ins, buffer);
        break;

    default:
        break;
    }
}

// I'm using fixed point notation, because the RP2040 has no FPU and these calculation bust be quick to compute

// signed [s][15].[16]
#define SFP_SHIFT 15
typedef int32_t fixedPoint_int;
#define SFP_FROM_UINT(x) ((fixedPoint_int)(x) << SFP_SHIFT)
#define SFP_TO_UINT16(x) (fixedPoint_int)(x >> SFP_SHIFT)
#define SFP_DIV(x, y) (fixedPoint_int)((x << SFP_SHIFT) / y)

// unsigned [16].[16]
typedef uint32_t fixedPoint_uint;
#define FP_SHIFT 16
#define FP_FROM_UINT(x) ((fixedPoint_uint)(x) << FP_SHIFT)
#define FP_MUL(x, y) (((x)*(y)) >> FP_SHIFT)
#define FP_DIV(x, y) ((x << FP_SHIFT)/y)

// unsigned [32].[32]
typedef uint32_t doubleFixedPoint_uint;
#define DFP_SHIFT 14
#define DFP_FROM_UINT(x) ((doubleFixedPoint_uint)(x) << DFP_SHIFT)
#define DFP_MUL(x, y) (((x)*(y)) >> DFP_SHIFT)
#define DFP_DIV(x, y) ((x << DFP_SHIFT)/y)
#define DFP_FROM_FP(x) ((doubleFixedPoint_uint)(x) << (DFP_SHIFT-FP_SHIFT))
#define DFP_TO_FP(x) ((doubleFixedPoint_uint)(x) >> (DFP_SHIFT-FP_SHIFT))

// More Complex generation function

void __gen_line(__instruction_t ins, uint16_t * buffer) {
    fixedPoint_int from = SFP_FROM_UINT(ins.param[PARAM_LINE_FROM_VALUE]);
    fixedPoint_int to   = SFP_FROM_UINT(ins.param[PARAM_LINE_TO_VALUE]);

    fixedPoint_int delta_y = to - from;
    fixedPoint_int delta_x = SFP_FROM_UINT(ins.length-1);
    volatile int32_t step = delta_y / delta_x;

    for(size_t i=0; i<ins.length; i++) {
        buffer[i*2] = SFP_TO_UINT16(SFP_FROM_UINT(i) * step);
    }
}

// This is to slow (39,6 ms for a full buffer per Channel) -> use of fixpoint notation
void __gen_quadratic_f(__instruction_t ins, uint16_t * buffer) {
    float x, y, c0, c1, c2;
    float p0 = (float)(ins.param[PARAM_QUADRATIC_P0])/0xffff;
    float p1 = (float)(ins.param[PARAM_QUADRATIC_P1])/0xffff;
    float p2 = (float)(ins.param[PARAM_QUADRATIC_P2])/0xffff;
    
    for(size_t i=0; i<ins.length; i++) {
        x = (float)(i)/ins.length;

        c2 = x*x;
        c0 = 1 - (x + x) + c2;
        c1 = (x + x) - (c2 + c2);
        y = p0*c0 + p1*c1 + p2*p2;

        buffer[i*2] = (uint16_t)((y)*0xffff);
    }
}

// This takes ~17.0ms to compute
void __gen_quadratic(__instruction_t ins, uint16_t * buffer) {
    // TODO: c0 is symmetric to c2, this can optimized further
    fixedPoint_uint x, y, c0, c1, c2;
    uint32_t p0 = ins.param[PARAM_QUADRATIC_P0];
    uint32_t p1 = ins.param[PARAM_QUADRATIC_P1];
    uint32_t p2 = ins.param[PARAM_QUADRATIC_P2];
    const uint32_t length = (uint32_t)(ins.length);

    for(uint32_t i=0; i<ins.length; i++) {
        x = FP_DIV(i, length);

        c2 = FP_MUL(x, x);
        c0 = FP_FROM_UINT(1) - (x + x) + c2;
        c1 = (x + x) - (c2 + c2);
        y =  FP_MUL(c0, p0) + FP_MUL(c1, p1) + FP_MUL(c2, p2);

        buffer[i*2] = (uint16_t)((y*0xffff) >> FP_SHIFT);
    }
}

// This takes ~17.0ms to compute
void __gen_cubic(__instruction_t ins, uint16_t * buffer) {
    // TODO: c0, c1 are symmetric to c2, c3, this can optimized further
    fixedPoint_uint x, x_inv, y, c0, c1, c2, c3, q;
    uint32_t p0 = ins.param[PARAM_CUBIC_P0];
    uint32_t p1 = ins.param[PARAM_CUBIC_P1];
    uint32_t p2 = ins.param[PARAM_CUBIC_P2];
    uint32_t p3 = ins.param[PARAM_CUBIC_P3];
    const uint32_t length = (uint32_t)(ins.length);

    for(uint32_t i=0; i<ins.length; i++) {
        x = FP_DIV(i, length);
        q = (FP_FROM_UINT(1) - 2*x + FP_MUL(x, x)); 
        c0 = FP_MUL(q, (FP_FROM_UINT(1) - x));
        c1 = FP_MUL(q, 3*x);

        x_inv = FP_DIV(length - i, length);
        q = (FP_FROM_UINT(1) - 2*x_inv + FP_MUL(x_inv, x_inv)); 
        c2 = FP_MUL(q, 3*x_inv);
        c3 = FP_MUL(q, (FP_FROM_UINT(1) - x_inv));

        y = FP_MUL(c0, p0) + FP_MUL(c1, p1) + FP_MUL(c2, p2) + FP_MUL(c3, p3);

        buffer[i*2] = (uint16_t)((y*0xffff) >> FP_SHIFT);
    }
}


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
    buffer[3 + 2*PARAM_LINE_FROM_VALUE    ] = (from  & 0xff00) >> 8;
    buffer[3 + 2*PARAM_LINE_FROM_VALUE + 1] = (from  & 0x00ff);
    buffer[3 + 2*PARAM_LINE_TO_VALUE      ] = (to    & 0xff00) >> 8;
    buffer[3 + 2*PARAM_LINE_TO_VALUE   + 1] = (to    & 0x00ff);
    return 7;
}

size_t add_ins_quadratic(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length,
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

size_t add_ins_cubic(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length,
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

