// By default, ESP32 runs a derivative of FreeRTOS to organize both developer
// code and background housekeeping work. Our apps can participate in this
// system as well, so this project serves as a playground for learning and
// experimentation of FreeRTOS API.
//
// Copyright (c) Roger Cheng and released "AS IS" under MIT License

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main()
{
  while(1)
  {
    printf("Verify build tool chain works\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}