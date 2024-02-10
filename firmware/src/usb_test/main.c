#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "hardware/timer.h"

#define LED 25

int main(int argc, char const *argv[]) {

    gpio_init(LED);
    gpio_set_dir(LED, true);

    while (true) {
        sleep_ms(500);
        gpio_put(LED, 1);
        sleep_ms(500);
        gpio_put(LED, 0);
    }

    return 0;
}
