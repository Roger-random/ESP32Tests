#include "duty_cycle.h"

void update_duty_cycle(float velocity, mcpwm_motor_control MC)
{
  if (velocity > 0)
  {
    mcpwm_set_duty(MC.unit, MC.timer, MCPWM_GEN_A, velocity);
    mcpwm_set_duty(MC.unit, MC.timer, MCPWM_GEN_B, 0);
  }
  else if (velocity < 0)
  {
    mcpwm_set_duty(MC.unit, MC.timer, MCPWM_GEN_A, 0);
    mcpwm_set_duty(MC.unit, MC.timer, MCPWM_GEN_B, fabs(velocity));
  }
  else
  {
    mcpwm_set_duty(MC.unit, MC.timer, MCPWM_GEN_A, 0);
    mcpwm_set_duty(MC.unit, MC.timer, MCPWM_GEN_B, 0);
  }
}

void duty_cycle_task(void* pvParam)
{
  joy_msg message;

  // Retrieve parameter: joystick message queue
  if (NULL == pvParam)
  {
    printf("ERROR: duty_cycle_task expected handle to joy_msg queue.\n");
    vTaskDelete(NULL); // Delete self.
  }
  QueueHandle_t xJoystickQueue = (QueueHandle_t)pvParam;

  // Initialize MCPWM peripherals
  float duty_cycle = 0.0;
  mcpwm_config_t mcpwm_config = {
    .frequency = pwm_freq,
    .cmpr_a = 0,
    .cmpr_b = 0,
    .counter_mode = MCPWM_UP_COUNTER,
    .duty_mode = MCPWM_DUTY_MODE_0,
  };
  for (int wheel = 0; wheel < wheel_count; wheel++)
  {
    mcpwm_init(
      speed_control[wheel].unit,
      speed_control[wheel].timer,
      &mcpwm_config);
    mcpwm_gpio_init(
      speed_control[wheel].unit,
      speed_control[wheel].signalA,
      speed_control[wheel].gpioA);
    mcpwm_gpio_init(
      speed_control[wheel].unit,
      speed_control[wheel].signalB,
      speed_control[wheel].gpioB);
    update_duty_cycle(duty_cycle, speed_control[wheel]);
  }

  // Enter interactive loop
  bool bWaitingForPress = true;
  bool bWaitingForCenter = false;
  bool bMotorOn = false;
  bool bUpdated = false;
  float fIncrement = 1.0;
  while(true)
  {
    bUpdated = false;
    if (pdTRUE == xQueueReceive(xJoystickQueue, &message, portMAX_DELAY))
    {
      if (bWaitingForPress && message.buttons[button_mode])
      {
        bWaitingForPress = false;
        if (bMotorOn)
        {
          bMotorOn = false;
        }
        else
        {
          bMotorOn = true;
        }
        bUpdated = true;
      }
      else if (!bWaitingForPress && !message.buttons[button_mode])
      {
        bWaitingForPress = true;
      }

      if (bWaitingForCenter &&
        fabs(message.axes[axis_speed]) < center_threshold &&
        fabs(message.axes[axis_steer]) < center_threshold)
      {
        bWaitingForCenter = false;
      }
      else if (!bWaitingForCenter &&
        fabs(message.axes[axis_speed]) > center_threshold)
      {
        bWaitingForCenter = true;
        if (message.axes[axis_speed] > 0)
        {
          duty_cycle += fIncrement;
          if (duty_cycle > 100.0)
          {
            duty_cycle = 100.0;
          }
        }
        else
        {
          duty_cycle -= fIncrement;
          if (duty_cycle < -100.0)
          {
            duty_cycle = -100.0;
          }
        }
        bUpdated = true;
      }
      else if (!bWaitingForCenter &&
        fabs(message.axes[axis_steer]) > center_threshold)
      {
        bWaitingForCenter = true;
        if (message.axes[axis_steer] > 0)
        {
          fIncrement *= 10;
          if (fIncrement > 10.0)
          {
            fIncrement = 10;
          }
        }
        else
        {
          fIncrement /= 10;
          if (fIncrement < 0.1)
          {
            fIncrement = 0.1;
          }
        }
        bUpdated = true;
      }

      if (bUpdated)
      {
        printf("Motor %d increment %.1f duty %.1f\n", bMotorOn, fIncrement, duty_cycle);
        for (int wheel = 0; wheel < wheel_count; wheel++)
        {
          if (bMotorOn)
          {
            update_duty_cycle(duty_cycle, speed_control[wheel]);
          }
          else
          {
            update_duty_cycle(0, speed_control[wheel]);
          }
        }
      }
    }
    else
    {
      printf("ERROR: duty_cycle_task queue receive failed.\n");
    }
  }
}
