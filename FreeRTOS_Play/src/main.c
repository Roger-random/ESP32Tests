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
#include "queue_print.h"
#include "status_led.h"

void app_main()
{
  xTaskCreate(status_led_task, "status_led_task", 2048, NULL, 10, NULL);

  QueueHandle_t xJoystickQueue = NULL;
  xJoystickQueue = xQueueCreate(1, sizeof(Joystick_t));
  if (NULL == xJoystickQueue)
  {
    printf("ERROR: Joystick data queue allocation failed.");
  }
  else
  {
    xTaskCreate(joystick_read_task, "joystick_read_task", 2048, xJoystickQueue, 15, NULL);
    xTaskCreate(queue_print_task, "queue_print_task", 2048, xJoystickQueue, 14, NULL);
  }
}
