#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "status_led.h"

void app_main()
{
    xTaskCreate(status_led_task, "status_led_task", 2048, NULL, 10, NULL);
}
