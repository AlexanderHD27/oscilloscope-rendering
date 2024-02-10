#include "pico/stdlib.h"

#define LED 25

int main(int argc, char const *argv[]) {

    gpio_init(LED);
    gpio_set_dir(LED, 1);

    while (true) {
        sleep_ms(500);
        gpio_put(LED, 1);
        sleep_ms(500);
        gpio_put(LED, 0);
    }

    return 0;
}
