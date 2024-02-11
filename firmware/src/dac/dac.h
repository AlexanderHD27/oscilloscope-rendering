#ifndef DAC_CUSTOM
#define DAC_CUSTOM

#include "pico/types.h"
#include "hardware/pio.h"

#define PIO_CLK_DIV(freq) (SYS_CLK_MHZ*1000000)/(freq)

#define CONV_NORMAL_TO_UINT16(x) (uint16_t)( 0xffff*((x+1)/2) )

#define PI 3.14159265359

// Driver Stuff
typedef enum {
    A, B
} BUF_SEL;

void init_dac_driver(
    PIO pio, uint sm, uint data_pin_start, 
    uint control_pin_start, 
    uint16_t (*buffer_a)[], uint16_t (*buffer_b)[], size_t buffer_size,
    void (*handle_next_frame )(BUF_SEL)
);


// Util function for generating some common wave forms

void pregen_calibration_cross(__uint16_t * buffer, size_t buffer_size);
void gen_sins(__uint16_t * buffer, size_t buffer_size, float freq_x, float freq_y, float phase_x, float phase_y, float amp_x, float y_amp);
void pregen_chirp(__uint16_t * buffer, size_t buffer_size);
void pregen_rect(__uint16_t * buffer, size_t buffer_size, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void pregen_square_wave(__uint16_t * buffer, size_t buffer_size);

#endif