#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <joy_adc.h>
#include <joy_msg.h>

void app_main()
{
  QueueHandle_t xJoystickQueue = xQueueCreate(1, sizeof(joy_msg));

  if (NULL == xJoystickQueue)
  {
    printf("ERROR: Queue allocation failed.\n");
  }
  else
  {
    xTaskCreate(joy_adc_read_task, "joy_adc_read_task", 2048, xJoystickQueue, 20, NULL);
  }
};