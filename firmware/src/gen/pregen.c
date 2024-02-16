/**
 * @file pregen.c
 * @author AlexanderHD27
 * @brief Contains function that pre-fill a buffer, with different types of signal
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 * None the functions in this fill are optimized and cannot be used for real-time-processing
 * 
 */
#include "pico/stdlib.h"
#include <stdlib.h>
#include <math.h>

#include "dac.h"

void gen_pregenCalibrationCross(__uint16_t * buffer, size_t buffer_size) {
    uint16_t y0 = 0;
    uint16_t y1 = 0;
    volatile int x_sec = 0;
    volatile int x_sub = 0;

    const uint div = 8;
    const uint sub_x_range = buffer_size/div;

    float x_sin = 0.0;
    for(int i=0; i<buffer_size; i++) {
        x_sub = i % sub_x_range;
        x_sin = (((float)x_sub)/sub_x_range * 2*PI);
        x_sec = i / sub_x_range;
    
        y0 = (uint16_t)((sinf(x_sin)*0.5 + 0.5) * 0x7fff) + 0xffff/4;
        y1 = (uint16_t)((sinf(x_sin + 0.5*PI)*0.5 + 0.5) * 0x7fff) + 0xffff/4;

        if(x_sec % 2) {
            buffer[i*2] = y0;
            buffer[i*2 + 1] = y1;
        } else {
            buffer[i*2] = x_sec % 8 == 2 ? (((float)x_sub)/sub_x_range) * 0xffff : 0x7fff;
            buffer[i*2 + 1] = x_sec % 8 == 0 ? (((float)x_sub)/sub_x_range) * 0xffff : 0x7fff;
        }
    }
}


void gen_pregenSine(__uint16_t * buffer, size_t buffer_size, 
    float freq_x, float freq_y, 
    float phase_x, float phase_y, 
    float amp_x, float y_amp
) {
    // TODO: Implement dac_pregenSine()
}

void gen_pregenChirp(uint16_t * buffer, size_t buffer_size) {
    float steps = UINT16_MAX / buffer_size;
    uint16_t x = 0;

    for(size_t i=0; i<buffer_size; i++) {
        x = (uint16_t)(steps*i);
        buffer[i*2] = x;
        buffer[i*2 + 1] = x; 
    }
}

void gen_pregenRect(__uint16_t * buffer, size_t buffer_size,
    uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1
) {
    // 4/5 Line, 1/5 Point
    const size_t segment_duration = buffer_size/4;
    const size_t point_duration = 1*(segment_duration/5);
    const size_t line_duration =  segment_duration - point_duration;

    const size_t chunk_remainder = buffer_size - segment_duration*4;
    const float stepX = (x1 - x0) / line_duration;
    const float stepY = (y1 - y0) / line_duration;

    // Lines
    for(size_t i=0; i<line_duration; i++) {
        // x0, y0 -> x1, y0
        buffer[(point_duration*1 + line_duration*0 + i)*2    ] = x0 + stepX*i;
        buffer[(point_duration*1 + line_duration*0 + i)*2 + 1] = y0; 
        // x1, y0 -> x1, y1
        buffer[(point_duration*2 + line_duration*1 + i)*2    ] = x1;
        buffer[(point_duration*2 + line_duration*1 + i)*2 + 1] = y0 + stepY*i; 
        // x1, y1 -> x0, y1
        buffer[(point_duration*3 + line_duration*2 + i)*2    ] = x1 - stepX*i;
        buffer[(point_duration*3 + line_duration*2 + i)*2 + 1] = y1; 
        // x0, y1 -> x0, y0
        buffer[(point_duration*4 + line_duration*3 + i)*2    ] = x0;
        buffer[(point_duration*4 + line_duration*3 + i)*2 + 1] = y1 - stepY*i; 
    }

    for(int i=0; i<point_duration; i++) {
        // x0, y0
        buffer[(segment_duration*0 + i)*2    ] = x0;
        buffer[(segment_duration*0 + i)*2 + 1] = y0; 
        // x1, y0
        buffer[(segment_duration*1 + i)*2    ] = x1;
        buffer[(segment_duration*1 + i)*2 + 1] = y0; 
        // x1, y1
        buffer[(segment_duration*2 + i)*2    ] = x1;
        buffer[(segment_duration*2 + i)*2 + 1] = y1; 
        // x0, y1
        buffer[(segment_duration*3 + i)*2    ] = x0;
        buffer[(segment_duration*3 + i)*2 + 1] = y1; 
    }

    for(int i=0; i<chunk_remainder; i++) {
        buffer[(segment_duration*4 + i) * 2] = x0;
        buffer[(segment_duration*4 + i) * 2] = x1;
    }
}

void gen_pregenSquareWave(__uint16_t * buffer, size_t buffer_size) {
    uint16_t x;
    for(size_t i=0; i<buffer_size; i++) {
        x = 0xffff * (i & 0b1);
        buffer[i*2] = x;
        buffer[i*2 + 1] = x ^ 0xffff; 
    }
}
