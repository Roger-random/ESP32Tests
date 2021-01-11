// By default, ESP32 runs a derivative of FreeRTOS to organize both developer
// code and background housekeeping work. Our apps can participate in this
// system as well, so this project serves as a playground for learning and
// experimentation of FreeRTOS API.
//
// Copyright (c) Roger Cheng and released "AS IS" under MIT License

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const gpio_num_t led_pin = GPIO_NUM_14;

void app_main()
{
  // Configure a pin for GPIO output to blink an LED
  gpio_config_t io_conf = {
      .mode = GPIO_MODE_OUTPUT,
      .intr_type = GPIO_INTR_DISABLE,
      .pull_down_en = 0,
      .pull_up_en = 0,
      .pin_bit_mask = (1ULL<<led_pin),
  };
  gpio_config(&io_conf);

  // Blink that LED
  bool led_on = true;
  while(1)
  {
    gpio_set_level(led_pin, led_on);
    vTaskDelay(pdMS_TO_TICKS(1000));
    led_on = !led_on;
  }
}
