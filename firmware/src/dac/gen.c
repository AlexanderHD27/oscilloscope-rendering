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

#define PARAM_SIN_FROM 0
#define PARAM_SIN_TO 1
#define PARAM_SIN_PERIOD 2
#define PARAM_SIN_PHASE 3

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

    case SINE:
        __gen_sine_taylor(ins, buffer);
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
#define FP_FROM_FLOAT(x) ((fixedPoint_uint)(x * (1 << FP_SHIFT)))

// These two implementation of fixpoint worked in most case but sadly not in all
//define FP_MUL(x, y) (((x*y)) >> FP_SHIFT)
//define FP_DIV(x, y) ((x << FP_SHIFT)/y)

// This is more expensive, because we need more bits to calculate
#define FP_MUL(x, y) ((fixedPoint_uint)((((uint64_t)(x)) * ((uint64_t)(y))) >> FP_SHIFT))
#define FP_DIV(x, y) ((((uint64_t)(x)) << FP_SHIFT) / ((uint64_t)(y)))

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

// This is to slow -> use of fixpoint notation
// 79,2 ms for 4096 Samples (2 channels)
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

// This takes ~17.0ms to compute for 4096 Samples (2 Channels)
// See https://www.desmos.com/calculator/lqwrsq6pdb
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

// This takes ~17.0ms to compute for 4096 Samples (2 Channels)
// See https://www.desmos.com/calculator/usunemqqbi
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

// Using Bhaskra I's Approximation (Its bad)
// see https://www.desmos.com/calculator/3tu2pwap4c

// unsigned [19].[13]
#define FIX_POINT_SINE_SHIFT 13
#define FIX_POINT_SINE_CONV_SHIFT (FIX_POINT_SHIFT - FIX_POINT_SINE_SHIFT)

void __gen_sine(__instruction_t ins, uint16_t * buffer) {
    fixedPoint_uint x, y, a;
    uint32_t i_;
    const uint16_t period = ins.param[PARAM_SIN_PERIOD]; 
    const uint16_t phase = ins.param[PARAM_SIN_PHASE]; 
    const uint16_t half_period = period / 2;
    const uint32_t sub = ((0b1 << (FIX_POINT_SINE_SHIFT))-1);
    bool neg = false;

    for(uint32_t i=0; i<ins.length; i++) {
        i_ = i + phase;
        x = FP_DIV(i_%half_period, half_period);
        neg = (i_/half_period) & 0b1;
        
        a = 4*(x - FP_MUL(x, x)) >> FIX_POINT_SINE_CONV_SHIFT;
        
        y = (a<<FIX_POINT_SINE_SHIFT)/((5 << FIX_POINT_SINE_SHIFT) - a);
        y = neg ? y - sub : sub - y;
        y = y << FIX_POINT_SINE_CONV_SHIFT;

        buffer[i*2] = (uint16_t)((y*0xffff) >> FP_SHIFT);
    }
}

#define PRE_POWER_3(x) (x*x*x)
#define PRE_POWER_5(x) (x*x*x*x*x)
 
#define HALF_PI PI/2
#define SINE_FACTOR_1 FP_FROM_FLOAT(HALF_PI)
#define SINE_FACTOR_3 FP_FROM_FLOAT(PRE_POWER_3(HALF_PI)/(6))
#define SINE_FACTOR_5 FP_FROM_FLOAT(PRE_POWER_5(HALF_PI)/(120)) // This -0x12f is a correction factor, due to probably rounding errors

// This uses taylor-series to approx. a sin wave. Its reasonable accurate, but takes longer to computer
// see https://www.desmos.com/calculator/77trwqesdm
// This takes ~65.9ms to compute for 4096 Samples (2 Channels) (This is too slow)

void __gen_sine_taylor(__instruction_t ins, uint16_t * buffer) {
    fixedPoint_uint x, x2, x3, x5, c1, c3, c5, y_out;
    uint64_t y;

    // This clamping of the input range values is done so we don't hit an accidental overall (tested values)
    const uint32_t min_level = MIN(MAX(ins.param[PARAM_SIN_FROM], 0x00a0), 0xff60);
    const uint32_t max_level = MIN(MAX(ins.param[PARAM_SIN_TO], 0x00a0), 0xff60);

    const uint32_t amp = (max_level - min_level)/2;
    const uint32_t mid_point = amp + min_level;

    const uint16_t phase = ins.param[PARAM_SIN_PHASE];
    const uint16_t period = ins.param[PARAM_SIN_PERIOD]; 
    const uint32_t length = (uint32_t)(ins.length);

    volatile uint32_t i_quod_cycle, quadrant, period_offset, i;
    volatile const uint32_t quod_cycle = period / 4;

    for(uint32_t i_=0; i_<length; i_++) {
        i = i_;
        x = FP_DIV(FP_FROM_UINT((i*4) % period+1), FP_FROM_UINT(period-1)); // Convert to normal Value
        
        // Create Taylor-1/4-Cycle
        // TODO: This should be computable without uint64 
        x2 = FP_MUL(x, x);
        x3 = FP_MUL(x, x2);
        x5 = FP_MUL(x3, x2);
        
        c1 = FP_MUL(x,  SINE_FACTOR_1);
        c3 = FP_MUL(x3, SINE_FACTOR_3);
        c5 = FP_MUL(x5, SINE_FACTOR_5);
        y = c1 - c3 + c5; // This is also a normal value

        y_out = (FP_MUL(y, FP_FROM_UINT(amp)) >> 16);
        //y_out = (FP_MUL(x, FP_FROM_UINT(amp)) >> 16);

        // Flipping and inverting
        i_quod_cycle = i % period;
        quadrant = (i_quod_cycle)/(quod_cycle);
        period_offset = (i / period) * period;

        switch (quadrant) {
        case 0: buffer[2*(((period_offset + i_quod_cycle               ) + phase) % length)] = mid_point + y_out; break;
        case 1: buffer[2*(((period_offset + (3*quod_cycle-1) - i_quod_cycle) + phase) % length)] = mid_point + y_out; break;
        case 2: buffer[2*(((period_offset + i_quod_cycle               ) + phase) % length)] = mid_point - y_out; break;
        case 3: buffer[2*(((period_offset + (7*quod_cycle-1) - i_quod_cycle) + phase) % length)] = mid_point - y_out; break;
        }    
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

size_t add_ins_sine(uint8_t * buffer, enum INSTRUCTION_SEL_CHANNEL channel, uint16_t length, 
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

