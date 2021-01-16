#include <stdio.h>

#include "joystick.h"

void joystick_read_task(void* pvParameters)
{
  // Configure a pin for GPIO input to see if joystick button is pressed
  gpio_config_t io_conf = {
      .mode = GPIO_MODE_INPUT,
      .intr_type = GPIO_INTR_DISABLE,
      .pin_bit_mask = (1ULL<<joystick_button),
  };
  gpio_config(&io_conf);

  // Configure ADC to read joystick X and Y positions.
  adc1_config_width(joystick_precision);
  adc1_config_channel_atten(joystick_x, joystick_attenuation);
  adc1_config_channel_atten(joystick_y, joystick_attenuation);

  // Read loop
  int button_x = 0;
  int button_y = 0;
  int button_state = 0;
  while(true)
  {
    button_x = adc1_get_raw(joystick_x);
    button_y = adc1_get_raw(joystick_y);
    button_state = gpio_get_level(joystick_button);
    printf("Joystick: %d,%d @ %d\n", button_x, button_y, button_state);
    vTaskDelay(pdMS_TO_TICKS(joystick_read_period));
  }
}
