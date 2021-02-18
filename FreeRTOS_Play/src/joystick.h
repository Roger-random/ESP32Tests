#ifndef INC_JOYSTICK_H
#define INC_JOYSTICK_H

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "driver/gpio.h"

#include "soc/adc_channel.h"

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html
//
// ESP32 analog read configuration isn't "Set GPIO [X] to analog input mode"
// but indirectly configured via ADC channels which correspond to certain
// pins that we have to look up in documentation. Fortunately we have GPIO
// lookup macros to help.
//
// ADC2 is shared with WiFi and few other hardware peripherals. Given its
// constraints, usage is usually avoided.
//
// ADC1 does not have those constraints, and the input-only GPIO 34, 35, 36,
// and 39 correspond to ADC1. So those are our first choices for joystick input.
//
// "GPIO34-39 can only be set as input mode and do not have software pullup or pulldown functions."
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html

static const adc1_channel_t joystick_x = ADC1_GPIO36_CHANNEL;
static const adc1_channel_t joystick_y = ADC1_GPIO39_CHANNEL;
static const gpio_num_t     joystick_button = GPIO_NUM_34;

// Unknown: does setting resolution to lower resolution gain any benefit?
// Could a read operation be faster for fewer bits of precision?
// Available widths are 9 through 12. 2^9 = 512 and plenty for this app.
static const adc_bits_width_t joystick_precision = ADC_WIDTH_BIT_9;

// ADC measures up to ~800mV. But we can add attenuation to extend range
// of measurement. DB11 gets us up to ~2600mV. Since the joysticks are
// potentiometers between 3.3V and GND, we lose the range between 2.6 and 3.3.
// It doesn't matter much on these crappy lowest-bidder joysticks with barely
// any analog proportionality, so ignore for this app.
static const adc_atten_t    joystick_attenuation = ADC_ATTEN_DB_11;

// Amount of time to wait between reads, in milliseconds.
static const int            joystick_read_period   = 20;

// Joystick data will be distributed via a FreeRTOS queue
// * Caller is responsible for allocating a queue for this data type and
//   sending the handle as parameter to joystick_read_task().
// * joystick_read_task() will periodically call xQueueOverwrite() on the head
//   of the queue with latest data.
// * Anyone with handle to the queue can call xQueuePeek() to see latest data.
//
// joystick_read_task() does not care if there is one reader, multiple readers,
// or no readers at all. So this message is kept as lean as possible without
// any processing. (Filtering, averaging, scaling, etc.) This way we ensure no
// time is wasted calculating data nobody will read.
typedef struct xJoystickMessage
{
  TickType_t  timeStamp;
  uint32_t    uiX;
  uint32_t    uiY;
  bool        buttonUp;
} Joystick_t;

// FreeRTOS task which will read joystick data every joystick_read_period and
// posts to the given queue of type Joystick_t.
void joystick_read_task(void*);

#endif // #ifndef INC_JOYSTICK_H
