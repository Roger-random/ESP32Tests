#include "status_led.h"

void status_led_task(void *arg)
{
  // Configure a pin for GPIO output to blink an LED
  gpio_config_t io_conf = {
      .mode = GPIO_MODE_OUTPUT,
      .intr_type = GPIO_INTR_DISABLE,
      .pull_down_en = 0,
      .pull_up_en = 0,
      .pin_bit_mask = (1ULL<<led_pin),
  };
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  // Blink that LED
  while(1)
  {
    ESP_ERROR_CHECK(gpio_set_level(led_pin, true));
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_ERROR_CHECK(gpio_set_level(led_pin, false));
    vTaskDelay(pdMS_TO_TICKS(1990));
  }
}
