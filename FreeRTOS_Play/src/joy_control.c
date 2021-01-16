#include "joy_control.h"

int32_t joystick_range_normalization(uint32_t uiRaw, uint32_t uiMid, uint32_t uiMax, uint32_t uiNull)
{
  int32_t iNormalized = 0;

  if (uiRaw < uiMid-uiNull)
  {
    iNormalized = ((uiRaw*100)/(uiMid-uiNull))-100;
  }
  else if (uiRaw > uiMid+uiNull)
  {
    iNormalized = ((uiRaw-uiMid-uiNull)*100)/(uiMax-uiMid-uiNull);
  }

  return iNormalized;
}

void joy_control_task(void* pJoyControlTaskParameter)
{
  Joystick_t joyData;
  TickType_t tickCount;
  uint32_t   uiXmid, uiXnull, uiXmax; // Assumption: min is always zero
  uint32_t   uiYmid, uiYnull, uiYmax; // Assumption: min is always zero
  int32_t    iX, iXold;
  int32_t    iY, iYold;
  bool       buttonOld;

  // Retrieve handles to data input/output queues
  if (NULL == pJoyControlTaskParameter)
  {
    printf("ERROR: joy_control_task parameter is null. Expected reference to JoyControlTaskParameter_t");
    vTaskDelete(NULL); // Delete self.
  }
  JoyControlTaskParameter_t *pParam = (JoyControlTaskParameter_t*)pJoyControlTaskParameter;
  QueueHandle_t xJoystickQueue = pParam->xJoystickQueue;

  // Retrieve the first new joystick position posted after our startup, this
  // position is treated as center. Power cycle to re-center the joystick.
  // Which will need to be done often for poor quality joysticks that drift.
  if (pdTRUE != xQueueReceive(xJoystickQueue, &joyData, pdMS_TO_TICKS(1000)))
  {
    printf("ERROR: joy_control_task did not receive joystick data within one second of startup.");
    vTaskDelete(NULL); // Delete self.
  }

  tickCount = joyData.timeStamp;
  uiXmid = joyData.uiX;
  uiYmid = joyData.uiY;
  iXold = uiXmid;
  iYold = uiYmid;
  buttonOld = joyData.buttonUp;

  // Initial maximum values are double the presumably mid positions, will adjust as we go.
  uiXmax = uiXmid*2;
  uiYmax = uiYmid*2;

  // Size of null zone
  uiXnull = uiXmid / 20;
  uiYnull = uiYmid / 20;

  while(true)
  {
    if (pdTRUE == xQueuePeek(xJoystickQueue, &joyData, 0))
    {
      if (joyData.timeStamp == tickCount)
      {
        if (xTaskGetTickCount() > tickCount+pdMS_TO_TICKS(2500))
        {
          // TODO: handle xTaskGetTickCount() overflow to zero.
          printf("ERROR: No joystick data for some time, should probably stop.");
        }
      }
      else
      {
        if (joyData.uiX > uiXmax)
        {
          uiXmax = joyData.uiX;
        }
        iX = joystick_range_normalization(joyData.uiX, uiXmid, uiXmax, uiXnull);

        if (joyData.uiY > uiYmax)
        {
          uiYmax = joyData.uiY;
        }
        iY = joystick_range_normalization(joyData.uiY, uiYmid, uiYmax, uiYnull);

        if (iX != iXold || iY != iYold || joyData.buttonUp != buttonOld)
        {
          printf("joy_control (%d, %d)", iX, iY);

          if (joyData.buttonUp)
          {
            printf("\n");
          }
          else
          {
            printf(" pressed\n");
          }

          iXold = iX;
          iYold = iY;
          buttonOld = joyData.buttonUp;
        }

        tickCount = joyData.timeStamp;
      }
    }
    else
    {
      // Expected to happen at startup because we dequeued an item for
      // centering data and won't have anything until a new one is put in the
      // queue.
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

