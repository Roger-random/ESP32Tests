#include "joystick.h"

void joystick_read_task(void* pvParameter)
{
  // Get ready to use caller-allocated queue for communicating joystick data
  QueueHandle_t xJoystickQueue;
  if (NULL == pvParameter)
  {
    printf("ERROR: joystick_read_task parameter is null. Expected handle to joystick data queue.");
    vTaskDelete(NULL); // Delete self.
  }
  xJoystickQueue = (QueueHandle_t)pvParameter;

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
  Joystick_t joystickData;
  while(true)
  {
    joystickData.timeStamp = xTaskGetTickCount();
    joystickData.uiX = adc1_get_raw(joystick_x);
    joystickData.uiY = adc1_get_raw(joystick_y);
    joystickData.buttonUp = gpio_get_level(joystick_button);

    xQueueOverwrite(xJoystickQueue, &joystickData);
    vTaskDelay(pdMS_TO_TICKS(joystick_read_period));
  }
}
