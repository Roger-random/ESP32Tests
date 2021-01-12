#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

// My ESP32 board has a LED wired to GPIO_NUM_2
// Official Espressif ESP32 DevKitC doesn't show one on schematic.
// https://dl.espressif.com/dl/schematics/esp32_devkitc_v4-sch.pdf
static const gpio_num_t led_pin = GPIO_NUM_2;

void status_led_task(void*);
