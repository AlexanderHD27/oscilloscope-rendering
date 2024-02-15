#include "pico/stdlib.h"
#include "gen.h"

// This contains function regarding generation of different signals (sine, cubic, quadratic, line, const)

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

// These two implementation of fixpoint multiplication/division worked in most case but sadly not in all (e.g. Taylor-Approximation)
#define FP_MUL_UINT32(x, y) (((x*y)) >> FP_SHIFT)
#define FP_DIV_UINT32(x, y) ((x << FP_SHIFT)/y)

// This is more expensive, because we need more bits to calculate
#define FP_MUL(x, y) ((fixedPoint_uint)((((uint64_t)(x)) * ((uint64_t)(y))) >> FP_SHIFT))
#define FP_DIV(x, y) ((((uint64_t)(x)) << FP_SHIFT) / ((uint64_t)(y)))

// More Complex generation function
void gen_generateLine(instruction_t ins, uint16_t * buffer) {
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
void gen_generateQuadraticFloat(instruction_t ins, uint16_t * buffer) {
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
void gen_generateQuadratic(instruction_t ins, uint16_t * buffer) {
    // TODO: c0 is symmetric to c2, this can optimized further
    fixedPoint_uint x, y, c0, c1, c2;
    uint32_t p0 = ins.param[PARAM_QUADRATIC_P0];
    uint32_t p1 = ins.param[PARAM_QUADRATIC_P1];
    uint32_t p2 = ins.param[PARAM_QUADRATIC_P2];
    const uint32_t length = (uint32_t)(ins.length);

    for(uint32_t i=0; i<ins.length; i++) {
        x = FP_DIV_UINT32(i, length);

        c2 = FP_MUL_UINT32(x, x);
        c0 = FP_FROM_UINT(1) - (x + x) + c2;
        c1 = (x + x) - (c2 + c2);
        y =  FP_MUL_UINT32(c0, p0) + FP_MUL_UINT32(c1, p1) + FP_MUL_UINT32(c2, p2);

        buffer[i*2] = (uint16_t)((y*0xffff) >> FP_SHIFT);
    }
}

// This takes ~17.0ms to compute for 4096 Samples (2 Channels)
// See https://www.desmos.com/calculator/usunemqqbi
void gen_generateCubic(instruction_t ins, uint16_t * buffer) {
    // TODO: c0, c1 are symmetric to c2, c3, this can optimized further
    fixedPoint_uint x, x_inv, y, c0, c1, c2, c3, q;
    uint32_t p0 = ins.param[PARAM_CUBIC_P0];
    uint32_t p1 = ins.param[PARAM_CUBIC_P1];
    uint32_t p2 = ins.param[PARAM_CUBIC_P2];
    uint32_t p3 = ins.param[PARAM_CUBIC_P3];
    const uint32_t length = (uint32_t)(ins.length);

    for(uint32_t i=0; i<ins.length; i++) {
        x = FP_DIV_UINT32(i, length);
        q = (FP_FROM_UINT(1) - 2*x + FP_MUL_UINT32(x, x)); 
        c0 = FP_MUL_UINT32(q, (FP_FROM_UINT(1) - x));
        c1 = FP_MUL_UINT32(q, 3*x);

        x_inv = FP_DIV(length - i, length);
        q = (FP_FROM_UINT(1) - 2*x_inv + FP_MUL(x_inv, x_inv)); 
        c2 = FP_MUL_UINT32(q, 3*x_inv);
        c3 = FP_MUL_UINT32(q, (FP_FROM_UINT(1) - x_inv));

        y = FP_MUL_UINT32(c0, p0) + FP_MUL_UINT32(c1, p1) + FP_MUL_UINT32(c2, p2) + FP_MUL_UINT32(c3, p3);

        buffer[i*2] = (uint16_t)((y*0xffff) >> FP_SHIFT);
    }
}

// Using Bhaskra I's Approximation (Its bad)
// see https://www.desmos.com/calculator/3tu2pwap4c

// unsigned [19].[13]
#define FIX_POINT_SINE_SHIFT 13
#define FIX_POINT_SINE_CONV_SHIFT (FP_SHIFT - FIX_POINT_SINE_SHIFT)

void gen_generateSineBhaskra(instruction_t ins, uint16_t * buffer) {
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

void gen_generateSineTaylor(instruction_t ins, uint16_t * buffer) {
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
