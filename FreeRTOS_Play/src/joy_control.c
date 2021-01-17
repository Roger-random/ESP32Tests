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
  Chassis_t  chassisCommand;
  uint32_t   uiXmid, uiXnull, uiXmax; // Assumption: min is always zero
  uint32_t   uiYmid, uiYnull, uiYmax; // Assumption: min is always zero
  int32_t    iX;
  int32_t    iY;

  // Retrieve handles to data input/output queues
  if (NULL == pJoyControlTaskParameter)
  {
    printf("ERROR: joy_control_task parameter is null. Expected reference to JoyControlTaskParameter_t");
    vTaskDelete(NULL); // Delete self.
  }
  JoyControlTaskParameter_t *pParam = (JoyControlTaskParameter_t*)pJoyControlTaskParameter;
  QueueHandle_t xJoystickQueue = pParam->xJoystickQueue;
  QueueHandle_t xChassisQueue = pParam->xChassisQueue;

  // Retrieve the first new joystick position posted after our startup.
  if (pdTRUE != xQueueReceive(xJoystickQueue, &joyData, pdMS_TO_TICKS(1000)))
  {
    printf("ERROR: joy_control_task did not receive joystick data within one second of startup.");
    vTaskDelete(NULL); // Delete self.
  }

  // This first joystick position is treated as center. In order to re-center
  // the joystick, we'll need to stop the existing instance of this task and
  // start a new instance. This can be done by power cycling the ESP32 or via
  // FreeRTOS APIs.
  // Re-centering will need to be done occasionally for low quality joysticks
  // that drift, but not often enough to justifying cluttering up this
  // exploration code project.
  uiXmid = joyData.uiX;
  uiYmid = joyData.uiY;

  // Initial maximum values are double the presumably mid positions, will adjust as we go.
  uiXmax = uiXmid*2;
  uiYmax = uiYmid*2;

  // Size of null zone. When using good joysticks this can be smaller (1/50,
  // 1/100) and for bad joysticks this may need to be larger (1/10, 1/5).
  uiXnull = uiXmid / 20;
  uiYnull = uiYmid / 20;

  // Setup complete, start processing loop.
  while(true)
  {
    if (pdTRUE == xQueuePeek(xJoystickQueue, &joyData, 1000))
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

      chassisCommand.timeStamp = xTaskGetTickCount();
      chassisCommand.iMotorA = iX;
      chassisCommand.iMotorB = iY;
      chassisCommand.bBrake  = !joyData.buttonUp;
      xQueueOverwrite(xChassisQueue, &chassisCommand);
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

