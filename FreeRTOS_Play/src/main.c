// By default, ESP32 runs a derivative of FreeRTOS to organize both developer
// code and background housekeeping work. Our apps can participate in this
// system as well, so this project serves as a playground for learning and
// experimentation of FreeRTOS API.
//
// Copyright (c) Roger Cheng and released "AS IS" under MIT License

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joystick.h"
#include "joy_control.h"
#include "motor_l298n.h"
#include "status_led.h"

void app_main()
{
  xTaskCreate(status_led_task, "status_led_task", 2048, NULL, 10, NULL);

  QueueHandle_t xJoystickQueue = NULL;
  QueueHandle_t xChassisQueue = NULL;
  xJoystickQueue = xQueueCreate(1, sizeof(Joystick_t));
  xChassisQueue = xQueueCreate(1, sizeof(Chassis_t));
  if (NULL == xJoystickQueue)
  {
    printf("ERROR: Joystick data queue allocation failed.");
  }
  else if (NULL == xChassisQueue)
  {
    printf("ERROR: Chassis command queue allocation failed.");
  }
  else
  {
    xTaskCreate(joystick_read_task, "joystick_read_task", 2048, xJoystickQueue, 15, NULL);

    JoyControlTaskParameter_t joy_control_param = {
      .xJoystickQueue = xJoystickQueue,
      .xChassisQueue = xChassisQueue,
    };
    xTaskCreate(joy_control_task, "joy_control_task", 2048, &joy_control_param, 14, NULL);

    xTaskCreate(motor_l298n_task, "motor_l298n_task", 2048, xChassisQueue, 13, NULL);
  }
}
