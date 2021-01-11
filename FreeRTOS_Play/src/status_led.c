#include "status_led.h"

void status_led_task(void *arg)
{
  printf("status_led_task started");
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
    printf("status_led_task loop");
    gpio_set_level(led_pin, led_on);
    vTaskDelay(pdMS_TO_TICKS(500));
    led_on = !led_on;
  }
}
