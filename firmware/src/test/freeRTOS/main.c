#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define LED 25

void Blink(void *parm) {
    while (true) {
        gpio_put(LED, true);
        vTaskDelay(100);
        gpio_put(LED, false);
        vTaskDelay(100);    
    }    

}

TaskHandle_t blink_task;

void main() {
    stdio_init_all();
    gpio_init(LED);

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    uint32_t status = xTaskCreate(Blink, "blink", 512, NULL, tskIDLE_PRIORITY, &blink_task);
    vTaskStartScheduler();
}