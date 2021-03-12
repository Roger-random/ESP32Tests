/*
 *
 * Reads joystick input to dynamically adjust MCPWM duty cycle.
 * 
 * Copyright (c) Roger Cheng
 * Released AS-IS under MIT license
 * 
 */
#ifndef INC_DUTY_CYCLE_H
#define INC_DUTY_CYCLE_H

#include "math.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joy_msg.h"
#include "driver/mcpwm.h"

#define wheel_count 6

static const uint32_t pwm_freq = 20000;

static const float center_threshold = 0.5;

/*
 * @brief MCPWM peripheral parameters for a particular wheel
 */
typedef struct mcpwm_motor_control {
  mcpwm_unit_t unit;
  mcpwm_timer_t timer;
  mcpwm_io_signals_t signalA;
  gpio_num_t gpioA;
  mcpwm_io_signals_t signalB;
  gpio_num_t gpioB;
} mcpwm_motor_control;

/*
 * @brief MCPWM peripheral parameters for all wheels
 */
static const mcpwm_motor_control speed_control[wheel_count] = {
  // front_left
  {
    MCPWM_UNIT_0,
    MCPWM_TIMER_0,
    MCPWM0A,
    GPIO_NUM_32,
    MCPWM0B,
    GPIO_NUM_33,
  },
  // front_right
  {
    MCPWM_UNIT_1,
    MCPWM_TIMER_0,
    MCPWM0A,
    GPIO_NUM_19,
    MCPWM0B,
    GPIO_NUM_18,
  },
  // mid_left
  {
    MCPWM_UNIT_0,
    MCPWM_TIMER_1,
    MCPWM1A,
    GPIO_NUM_25,
    MCPWM1B,
    GPIO_NUM_26,
  },
  // mid_right
  {
    MCPWM_UNIT_1,
    MCPWM_TIMER_1,
    MCPWM1A,
    GPIO_NUM_17,
    MCPWM1B,
    GPIO_NUM_16,
  },
  // back_left
  {
    MCPWM_UNIT_0,
    MCPWM_TIMER_2,
    MCPWM2A,
    GPIO_NUM_27,
    MCPWM2B,
    GPIO_NUM_12,
  },
  // back_right
  {
    MCPWM_UNIT_1,
    MCPWM_TIMER_2,
    MCPWM2A,
    GPIO_NUM_4,
    MCPWM2B,
    GPIO_NUM_2,
  },
};

/*
 * @brief FreeRTOS task for interactively exploring MCPWM duty cycle
 * @param pvParam FreeRTOS QueueHandle_t for joy_msg queue
 */
void duty_cycle_task(void* pvParam);

#endif // #ifndef INC_DUTY_CYCLE_H
