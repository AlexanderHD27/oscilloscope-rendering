#include <stdlib.h>
#include <math.h>

#include "dac.h"
#include "pico/types.h"

/**
 * Pregenerate a circle + cross + point for calibration/testing. This should not be used for live-generation
 * @param buffer target buffer, that the signal is written to (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size how big is your buffer
*/
void gen_calibration_cross(__uint16_t * buffer, size_t buffer_size) {
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


/**
 * Generates two sin-waves with corrsponding parameters
 * @param buffer target buffer, that the signal is written to (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size how big is your buffer
 * @param freq_x frequency of the x-signal in 1/buffer_period
 * @param freq_y frequency of the y-signal in 1/buffer_period
 * @param phase_x phase-shift of the x-signal
 * @param phase_y phase-shift of the y-signal
 * @param amp_x amplitude of x-signal (1 = max signal strength)
 * @param amp_y amplitude of y-signal (1 = max signal strength)
*/
void gen_sins(__uint16_t * buffer, size_t buffer_size, 
    float freq_x, float freq_y, 
    uint16_t phase_x, uint16_t phase_y,
    float amp_x, float y_amp
) {
    // TODO: Implement gen_sins()
}

/**
 * Generates two chirps from -1 to +1 with a period of the buffer size
 * @param buffer target buffer, that the signal is written to (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size sample size of your buffer
*/
void gen_chirp(__uint16_t * buffer, size_t buffer_size) {
    // TODO: Implement gen_chirp()
}

/**
 * Generates a rectangle from (x0, y0) to (x1, y1)
 * @param buffer the target, that the signal is written two (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size sample size of your buffer
*/
void gen_rect(__uint16_t * buffer, size_t buffer_size,
    uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1
) {
    // TODO: Implement gen_rect()
}

/**
 * Generates a square at maximum frequency
 * @param buffer the target, that the signal is written two (note that the buffer needs at least size of 2*buffer_size)
 * @param buffer_size sample size of your buffer
*/
void gen_square_wave(__uint16_t * buffer, size_t buffer_size) {
    // TODO: Implement gen_square_wave()
}
