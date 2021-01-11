// By default, ESP32 runs a derivative of FreeRTOS to organize both developer
// code and background housekeeping work. Our apps can participate in this
// system as well, so this project serves as a playground for learning and
// experimentation of FreeRTOS API.
//
// Copyright (c) Roger Cheng and released "AS IS" under MIT License

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "status_led.h"

void app_main()
{
  xTaskCreate(status_led_task, "status_led_task", 1024, NULL, 10, NULL);
}
