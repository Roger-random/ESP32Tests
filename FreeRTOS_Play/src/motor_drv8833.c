#include "motor_drv8833.h"

void set_channel(bool bBrake, int32_t iSpeed, mcpwm_generator_t gen1, mcpwm_generator_t gen2)
{
  if (bBrake)
  {
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen1, 100);
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen2, 100);
  }
  else if (iSpeed > 0)
  {
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen1, abs(iSpeed) * duty_cycle_max / 100);
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen2, 0);
  }
  else if (iSpeed < 0)
  {
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen1, 0);
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen2, abs(iSpeed) * duty_cycle_max / 100);
  }
  else
  {
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen1, 0);
    mcpwm_set_duty(drv8833_mcpwm_unit, drv8833_mcpwm_timer, gen2, 0);
  }
}

void motor_drv8833_task(void* pvMotorTaskParameter)
{
  // Get queue to retrieve motor control commands
  QueueHandle_t xMotorControlQueue;
  if (NULL == pvMotorTaskParameter)
  {
    printf("ERROR: motor_l298n_task parameter is null. Expected handle to motor control message queue.\n");
    vTaskDelete(NULL); // Delete self.
  }
  xMotorControlQueue = (QueueHandle_t)pvMotorTaskParameter;

  // Set up MCWPM peripheral to control DRV8833 input pins
  mcpwm_gpio_init(drv8833_mcpwm_unit, MCPWM0A, drv8833_ain1);
  mcpwm_gpio_init(drv8833_mcpwm_unit, MCPWM0B, drv8833_ain2);
  mcpwm_gpio_init(drv8833_mcpwm_unit, MCPWM1A, drv8833_bin1);
  mcpwm_gpio_init(drv8833_mcpwm_unit, MCPWM1B, drv8833_bin2);
  mcpwm_config_t mcpwm_config = {
    .frequency = drv8833_mcpwm_freq,
    .cmpr_a = 0,
    .cmpr_b = 0,
    .counter_mode = MCPWM_UP_COUNTER,
    .duty_mode = MCPWM_DUTY_MODE_0,
  };
  mcpwm_init(drv8833_mcpwm_unit, drv8833_mcpwm_timer, &mcpwm_config);

  // DRV8833 control loop
  BaseType_t xResult;
  Chassis_t xMotorCommands;
  while(true)
  {
    // TODO: Track timestamp of last command and stop if timestamp hasn't changed in a while.
    xResult = xQueuePeek(xMotorControlQueue, &xMotorCommands, 0);
    if (pdTRUE == xResult)
    {
      //set_channel(xMotorCommands.bBrake, xMotorCommands.iMotorA, MCPWM_GEN_A, MCPWM_GEN_B);
      set_channel(xMotorCommands.bBrake, xMotorCommands.iMotorB, MCPWM_GEN_A, MCPWM_GEN_B);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
