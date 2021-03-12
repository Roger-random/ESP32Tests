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

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joy_msg.h"
#include "driver/mcpwm.h"

/*
 * @brief FreeRTOS task for interactively exploring MCPWM duty cycle
 * @param pvParam FreeRTOS QueueHandle_t for joy_msg queue
 */
void duty_cycle_task(void* pvParam);

#endif // #ifndef INC_DUTY_CYCLE_H
