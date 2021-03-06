
/*
 * Only lightly modified from ESP32 WiFi Station example
 * https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c
 */

#ifndef INC_STATION_START_H
#define INC_STATION_START_H
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "secrets.h"

/*
 * @brief FreeRTOS task to initialize ESP32 WiFi in station mode
 * @param pvParameter Currently unused
 */
void station_start_task(void* pvParameter);

/*
 * @brief Blocking call to initialize ESP32 WiFi in station mode
 */
esp_err_t station_start();

#endif // #ifndef INC_STATION_START_H