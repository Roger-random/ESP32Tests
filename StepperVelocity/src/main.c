// ESP32 pulse generation test intended to control velocity of a stepper motor.
// Does not track number of steps, and hence unsuitable for position control.
//
// Copyright (c) Roger Cheng and released "AS IS" under MIT License

// PWM generation code adapted from Espressif ESP-IDF LEDC PWM example
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/ledc/main/ledc_example_main.c

// Digital output code extracted from Espressif ESP-IDF GPIO example
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/gpio/generic_gpio/main/gpio_example_main.c

// Potentiometer reading code extracted from Espressif ESP-IDF ADC1 example
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/adc/main/adc1_example_main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_err.h"

// STEP and DIRECTION output pins for stepper motor driver.
static const gpio_num_t step_pin = GPIO_NUM_12;
static const gpio_num_t direction_pin = GPIO_NUM_14;

// Potentiometer input pin and values averaged over specified number of readings.
static const uint32_t multi_sample_count = 32;
static const adc_channel_t channel = ADC_CHANNEL_6; // Translates to GPIO_NUM_34 if used with ADC1

// ADC values as a result of using 12-bit width. Do not change unless changing ADC_WIDTH_BIT_12.
static const int32_t adc_min = 0;
static const int32_t adc_max = 4095;
static const int32_t adc_mid = (adc_max - adc_min)/2;

// PWM ranges as a result of timer resolution. Do not change unless changing from LEDC_TIMER_7_BIT.
static const int32_t freq_min = 8; // Hz
static const int32_t freq_max = 625000; // Hz

// Parameters for how potentiometer value is translated to motor velocity.
// Free to tune as appropriate for application. ("Season to taste")
static const int32_t speed_min = freq_min; // Anything lower than speed_min is treated as stopped
static const int32_t speed_max = 240; // Hz
static const int32_t accel_limit = 20; // Lower for more gradual changes.
static const bool    invert_direction = false;

void app_main(void)
{
    /////////////////////////////////////////////////////////////////////////
    //
    // Configure timer to be used by LEDC peripheral, then configure LEDC
    // to pulse our STEP pin.
    //
    // Only certain combinations of PWM frequency vs. duty cycle are allowed.
    // The higher the frequency, the coarser the control over duty cycle.
    // At 40MHz only 1-bit duty cycle control is possible: 0% 50%, or 100%.
    // At 20MHz, 2-bit, etc.
    //
    // This project desires a minimum PWM frequency of 8Hz, which imposes
    // a lower limit of 7-bits on duty cycle, which in turn imposes an
    // maximum PWM frequency of 625kHz.
    //
    //
    ledc_timer_config_t ledc_timer = {
        // Running Timer 0 in high speed mode. Not picky about which source
        // clock to use, so let it auto-select.
        .timer_num = LEDC_TIMER_0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .clk_cfg = LEDC_AUTO_CLK,

        // Desire minimum PWM frequency of 8Hz...
        .freq_hz = 8,

        // ... which means we have to use at least 7 bits on PWM duty cycle
        .duty_resolution = LEDC_TIMER_7_BIT,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        // Listening to beat of our drummer, high speed timer 0
        .timer_sel  = LEDC_TIMER_0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,

        // Details for this output
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0, // Start out stopped (0% duty cycle)
        .hpoint     = 0,
        .gpio_num   = step_pin,
    };
    ledc_channel_config(&ledc_channel);

    /////////////////////////////////////////////////////////////////////////
    //
    // Configure digital output for our DIRECTION pin.
    //  No interrupts will be driven by this pin
    //  No internal pull-down or pull-up resistors
    //
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .pin_bit_mask = (1ULL<<direction_pin),
    };
    gpio_config(&io_conf);

    /////////////////////////////////////////////////////////////////////////
    //
    // Configure input pin for analog-to-digital conversion (ADC) to read
    // potentiometer position.
    //  12-bit ADC = results will be between 0 and 4095 inclusive
    //  11 db attenuation allows reading the full range of 3.3V
    //
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_11);

    int32_t adc_cumulative;
    int32_t adc_average;
    int32_t  adc_zerocenter;

    int32_t speed_target = 0;
    int32_t speed_current = 0;
    int32_t speed_absolute = 0;

    bool direction_current = true;
    // Setup complete, enter infinite loop
    while (1) {
        // Since ADC is a noisy process, take multiple readings then average.
        adc_cumulative = 0;
        for (int32_t i = 0; i < multi_sample_count; i++) {
            adc_cumulative += adc1_get_raw(channel);
        }
        adc_average = adc_cumulative / multi_sample_count;

        // adc_zerocenter range: -adc_mid to +adc_mid
        adc_zerocenter = adc_average - adc_mid;

        // speed_target range: -speed_max to +speed_max
        speed_target = adc_zerocenter * speed_max / adc_mid;

        // Calculate new speed based on target and acceleration limit
        if (speed_target > speed_current + accel_limit) {
            speed_current += accel_limit;
        }
        else if (speed_target < speed_current - accel_limit) {
            speed_current -= accel_limit;
        }
        else {
            speed_current = speed_target;
        }

        printf("Speed current %d -- target %d\n", speed_current, speed_target);

        // Output direction pin
        direction_current = speed_current > 0;
        if (invert_direction) {
            direction_current = !direction_current;
        }
        gpio_set_level(direction_pin, direction_current);

        // Update PWM duty cycle and frequency
        speed_absolute = abs(speed_current);
        if (speed_absolute < speed_min) {
            // Deadband = PWM duty cycle zero. PWM frequency irrelevant.
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        } else {
            // 64 is 50% duty cycle in 7-bit PWM resolution.
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 64);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

            // Update PWM frequency
            if (speed_absolute > freq_max) {
                speed_absolute = freq_max;
            }
            ledc_set_freq(ledc_timer.speed_mode, ledc_timer.timer_num, speed_absolute);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
