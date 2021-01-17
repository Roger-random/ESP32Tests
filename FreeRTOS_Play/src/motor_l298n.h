#ifndef INC_MOTOR_L298N_H
#define INC_MOTOR_L298N_H
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joy_control.h"

// Pins connected to L298N control inputs.
// Not all ESP32 DevKit pins are usable. See Espressif documentation or
// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

static const gpio_num_t l298n_ena = GPIO_NUM_19;
static const gpio_num_t l298n_in1 = GPIO_NUM_18;
static const gpio_num_t l298n_in2 = GPIO_NUM_5;
static const gpio_num_t l298n_in3 = GPIO_NUM_17;
static const gpio_num_t l298n_in4 = GPIO_NUM_16;
static const gpio_num_t l298n_enb = GPIO_NUM_4;

/**
 * @brief Task that control motors using L298N based on control messages.
 * 
 * @param pvMotorTaskParameter handle to a queue of Chassis_t for motor
 * control message input.
 */

void motor_l298n_task(void*);

#endif // #ifndef INC_MOTOR_L298N_H
