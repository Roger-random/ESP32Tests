#include "esp_log.h"
#include "esp_system.h"

#include "station_start.h"

static const char *TAG = "MQTT test";

void app_main()
{
  ESP_LOGI(TAG, "Startup");
  station_start();
  ESP_LOGI(TAG, "Station started");
}