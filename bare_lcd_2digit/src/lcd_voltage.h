#ifndef INC_LCD_VOLTAGE_H
#define INC_LCD_VOLTAGE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

static const gpio_num_t seg_0 = GPIO_NUM_23;
static const gpio_num_t seg_1 = GPIO_NUM_22;
static const gpio_num_t seg_2 = GPIO_NUM_21;
static const gpio_num_t seg_3 = GPIO_NUM_19;
static const gpio_num_t seg_4 = GPIO_NUM_18;
static const gpio_num_t seg_5 = GPIO_NUM_5;
static const gpio_num_t seg_6 = GPIO_NUM_17;
static const gpio_num_t seg_7 = GPIO_NUM_16;

static const gpio_num_t com_0 = GPIO_NUM_4;
static const gpio_num_t com_1 = GPIO_NUM_15;

void lcd_voltage_task(void*);

#endif // INC_LCD_VOLTAGE_H
