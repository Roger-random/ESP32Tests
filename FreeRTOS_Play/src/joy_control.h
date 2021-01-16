#ifndef INC_JOY_CONTROL_H
#define INC_JOY_CONTROL_H
#include <stdio.h>
#include <float.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joystick.h"

typedef struct xJoyControlTaskParameter
{
  QueueHandle_t xJoystickQueue;
} JoyControlTaskParameter_t;

/**
 * @brief Task that generates control messages based on raw joystick data.
 * 
 * @param pJoyControlTaskParameter An instance of JoyControlTaskParameter_t
 * with handles to input and output message queues.
 */
void joy_control_task(void*);

#endif // #ifndef INC_JOY_CONTROL_H
