#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "station_start.h"
#include "http_file_server.h"

void app_main()
{
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  xTaskCreate(station_start_task, "station_start_task", 1024*3, NULL, 20, NULL);
  xTaskCreate(http_file_server_task, "http_file_server_task", 1024*4, NULL, 19, NULL);
}
