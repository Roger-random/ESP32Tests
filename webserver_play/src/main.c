#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "station_start.h"

void app_main()
{
  xTaskCreate(station_start_task, "station_start_task", 1024*8, NULL, 20, NULL);
}