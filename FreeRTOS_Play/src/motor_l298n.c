#include "motor_l298n.h"

void set_direction(int32_t iSpeed, gpio_num_t ioA, gpio_num_t ioB)
{
  if (iSpeed < 0)
  {
    gpio_set_level(ioA, 0);
    gpio_set_level(ioB, 1);
  }
  else
  {
    gpio_set_level(ioA, 1);
    gpio_set_level(ioB, 0);
  }
}

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

  // Set up digital output pins to signal IN1 through IN4
  uint64_t pin_bit_mask = 0;
  pin_bit_mask |= (1ULL<<l298n_in1);
  pin_bit_mask |= (1ULL<<l298n_in2);
  pin_bit_mask |= (1ULL<<l298n_in3);
  pin_bit_mask |= (1ULL<<l298n_in4);

  gpio_config_t io_conf = {
    .mode = GPIO_MODE_OUTPUT,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_down_en = 0,
    .pull_up_en = 0,
    .pin_bit_mask = pin_bit_mask,
  };
  gpio_config(&io_conf);

  // Set up MCPWM peripheral to generate PWM velocity control on ENA and ENB
  mcpwm_gpio_init(l298n_mcpwm_unit, MCPWM0A, l298n_ena);
  mcpwm_gpio_init(l298n_mcpwm_unit, MCPWM0B, l298n_enb);
  mcpwm_config_t mcpwm_config = {
    .frequency = l298n_mcpwm_freq,
    .cmpr_a = 0,
    .cmpr_b = 0,
    .counter_mode = MCPWM_UP_COUNTER,
    .duty_mode = MCPWM_DUTY_MODE_0, // ACTIVE = HIGH
  };
  mcpwm_init(l298n_mcpwm_unit, l298n_mcpwm_timer, &mcpwm_config);

  // L298N control loop
  BaseType_t xResult;
  Chassis_t xMotorCommands;
  while(true)
  {
    // TODO: stop motors if motor command timestamp hasn't been updated for a while.
    xResult = xQueuePeek(xMotorControlQueue, &xMotorCommands, 0);
    if (pdTRUE == xResult)
    {
      // Uncomment next line for diagnostic output
      // printf("Motor command A @ %d, B @ %d, Brake %d\n", xMotorCommands.iMotorA, xMotorCommands.iMotorB, xMotorCommands.bBrake);

      if (xMotorCommands.bBrake)
      {
        mcpwm_set_signal_high(l298n_mcpwm_unit, l298n_mcpwm_timer, MCPWM_OPR_A);
        gpio_set_level(l298n_in1, 1);
        gpio_set_level(l298n_in2, 1);
        mcpwm_set_signal_high(l298n_mcpwm_unit, l298n_mcpwm_timer, MCPWM_OPR_B);
        gpio_set_level(l298n_in3, 1);
        gpio_set_level(l298n_in4, 1);
      }
      else
      {
        // If we were in brake mode earlier, need to switch back to duty mode.
        mcpwm_set_duty_type(l298n_mcpwm_unit, l298n_mcpwm_timer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
        mcpwm_set_duty_type(l298n_mcpwm_unit, l298n_mcpwm_timer, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

        // Set duty cycle and direction for each L298N motor
        set_direction(xMotorCommands.iMotorA, l298n_in1, l298n_in2);
        mcpwm_set_duty(l298n_mcpwm_unit, l298n_mcpwm_timer, MCPWM_OPR_A, abs(xMotorCommands.iMotorA) * duty_cycle_max / 100);
        set_direction(xMotorCommands.iMotorB, l298n_in3, l298n_in4);
        mcpwm_set_duty(l298n_mcpwm_unit, l298n_mcpwm_timer, MCPWM_OPR_B, abs(xMotorCommands.iMotorB) * duty_cycle_max / 100);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}