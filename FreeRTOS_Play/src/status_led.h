#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

static const gpio_num_t led_pin = GPIO_NUM_2;

void status_led_task(void*);
