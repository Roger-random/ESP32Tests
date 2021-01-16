#include "queue_print.h"

void queue_print_task(void* pvParameter)
{
  // Get ready to use caller-allocated queue for communicating joystick data
  QueueHandle_t xJoystickQueue;
  if (NULL == pvParameter)
  {
    printf("ERROR: queue_print_task parameter is null. Expected handle to joystick data queue.");
    vTaskDelete(NULL); // Delete self.
  }
  xJoystickQueue = (QueueHandle_t)pvParameter;
  
  Joystick_t joyData;
  while(true)
  {
    if (pdTRUE == xQueuePeek(xJoystickQueue, &joyData, 0))
    {
      printf("%d retrieved Joystick at %d (%d,%d) button %d\n", xTaskGetTickCount(), joyData.timeStamp, joyData.uiX, joyData.uiY, joyData.buttonUp);
    }
    else
    {
      printf("No joystick data in queue.\n");
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
