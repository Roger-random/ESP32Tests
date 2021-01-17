#include "motor_l298n.h"

void motor_l298n_task(void* pvMotorTaskParameter)
{
  // Get queue to retrieve motor control commands
  QueueHandle_t xMotorControlQueue;
  if (NULL == pvMotorTaskParameter)
  {
    printf("ERROR: motor_l298n_task parameter is null. Expected handle to motor control message queue.\n");
    vTaskDelete(NULL); // Delete self.
  }
  xMotorControlQueue = (QueueHandle_t)pvMotorTaskParameter;

  BaseType_t xResult;
  Chassis_t xMotorCommands;

  while(true)
  {
    // TODO: stop motors if motor command timestamp hasn't been updated for a while.
    xResult = xQueuePeek(xMotorControlQueue, &xMotorCommands, 0);
    if (pdTRUE == xResult)
    {
      printf("Motor command A @ %d, B @ %d, Brake %d\n", xMotorCommands.iMotorA, xMotorCommands.iMotorB, xMotorCommands.bBrake);
    }

    vTaskDelay(pdMS_TO_TICKS(250));
  }
}