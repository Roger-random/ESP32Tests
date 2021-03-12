#include "duty_cycle.h"

void duty_cycle_task(void* pvParam)
{
  joy_msg message;

  if (NULL == pvParam)
  {
    printf("ERROR: duty_cycle_task expected handle to joy_msg queue.\n");
    vTaskDelete(NULL); // Delete self.
  }
  QueueHandle_t xJoystickQueue = (QueueHandle_t)pvParam;

  bool bWaitingForPress = true;
  while(true)
  {
    if (pdTRUE == xQueueReceive(xJoystickQueue, &message, portMAX_DELAY))
    {
      if (bWaitingForPress && message.buttons[button_mode])
      {
        bWaitingForPress = false;
        printf("Wax On!\n");
      }
      else if (!bWaitingForPress && !message.buttons[button_mode])
      {
        bWaitingForPress = true;
        printf("Wax Off!\n");
      }
    }
    else
    {
      printf("ERROR: duty_cycle_task queue receive failed.\n");
    }
  }
}
