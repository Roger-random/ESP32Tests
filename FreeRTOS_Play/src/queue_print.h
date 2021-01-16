#ifndef INC_QUEUE_PRINT_H
#define INC_QUEUE_PRINT_H

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "joystick.h"

// Task that periodically prints the latest data in Joystick queue, whose
// handle is given as parameter.
void queue_print_task(void*);

#endif // #ifndef INC_QUEUE_PRINT_H
