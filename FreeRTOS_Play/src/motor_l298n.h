#ifndef INC_MOTOR_L298N_H
#define INC_MOTOR_L298N_H
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joy_control.h"

/**
 * @brief Task that control motors using L298N based on control messages.
 * 
 * @param pvMotorTaskParameter handle to a queue of Chassis_t for motor
 * control message input.
 */

void motor_l298n_task(void*);

#endif // #ifndef INC_MOTOR_L298N_H
