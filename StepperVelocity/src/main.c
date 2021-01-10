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
static const int32_t multi_sample_count = 32;
static const adc_channel_t channel = ADC_CHANNEL_6; // Translates to GPIO_NUM_34 if used with ADC1

// ADC values as a result of using 12-bit width. Do not change unless changing ADC_WIDTH_BIT_12.
static const int32_t adc_min = 0;
static const int32_t adc_max = 4095;
static const int32_t adc_mid = (adc_max - adc_min)/2;

// PWM ranges as a result of timer resolution. Do not change unless changing from LEDC_TIMER_5_BIT.
static const int32_t freq_min = 32; // Hz
static const int32_t freq_max = 32500; // Hz

// Parameters for how potentiometer value is translated to motor velocity.
// Free to tune as appropriate for application. ("Season to taste")
static const int32_t adc_deadband = 100; // Stop if ADC value is within adc_mid +/- adc_deadband
static const int32_t speed_min = freq_min; // Hz. Requires: freq_min <= speed_min < speed_max
static const int32_t speed_max = 19000; // Hz Requires: speed_min < speed_max <= freq_max
static const int32_t update_period = 100; // milliseconds to wait between updates
static const int32_t accel_limit = 2000; // Hz. Speed change per update will not exceed this amount
static const bool    invert_direction = false;

// Values resulting from above parameters, should never need to change directly.
static const int32_t adc_motion = adc_mid - adc_deadband; // Range of ADC values producing motion
static const int32_t speed_range = speed_max - speed_min; // Valid range of speed in Hz

void app_main(void)
{
    /////////////////////////////////////////////////////////////////////////
    //
    // Parameter checking
    //
    if (adc_max < adc_min) {
        printf("INVALID PARAMETER: ADC max %d is less than min %d\n", adc_max, adc_min);
        return;
    }
    if (speed_min < freq_min) {
        printf("INVALID PARAMETER: Specified min speed %d below PWM minimum %d\n", speed_min, freq_min);
        return;
    }
    if (speed_min > speed_max) {
        printf("INVALID PARAMETER: Specified min speed %d exceeds specified top speed %d\n", speed_min, speed_max);
        return;
    }
    if (speed_max > freq_max) {
        printf("INVALID PARAMETER: Specified top speed %d exceeds PWM maximum %d\n", speed_max, freq_max);
        return;
    }


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
    ledc_timer_config_t ledc_timer = {
        // Running Timer 0 in high speed mode. Not picky about which source
        // clock to use, so let it auto-select.
        .timer_num = LEDC_TIMER_0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .clk_cfg = LEDC_AUTO_CLK,

        .freq_hz = 32,
        .duty_resolution = LEDC_TIMER_5_BIT,
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

    /////////////////////////////////////////////////////////////////////////
    //
    //  Variables we'll need for calculation during main loop
    //
    int32_t adc_cumulative; // Accumulated value of multiple ADC samples
    int32_t adc_average;    // Average of multiple ADC samples [adc_min, adc_max]
    int32_t adc_zerocenter; // ADC range centering at zero [-adc_mid, adc_mid]
    int32_t adc_absolute;   // Absolute value of zero-centered ADC range [0, adc_mid]

    int32_t speed_target = 0;   // Speed targeted by ADC
    int32_t speed_current = 0;  // Speed within acceleration limites sent to PWM.

    bool direction_current = true; // Movement direction

    /////////////////////////////////////////////////////////////////////////
    //
    //  Preparation complete, start main loop
    //
    while (1) {
        // Since ADC is a noisy process, take multiple readings.
        adc_cumulative = 0;
        for (int32_t i = 0; i < multi_sample_count; i++) {
            adc_cumulative += adc1_get_raw(channel);
        }

        // Then take the average of all those values.
        adc_average = adc_cumulative / multi_sample_count;

        // Center range of values at zero.
        adc_zerocenter = adc_average - adc_mid;

        // For deadband & scaling math, throw away negative sign.
        adc_absolute = abs(adc_zerocenter);

        // Target speed is zero within deadband. Outside of deadband, it is
        // scaled to range [speed_min, speed_max]
        if (adc_absolute < adc_deadband) {
            speed_target = 0;
        } else {
            adc_absolute -= adc_deadband; // Range now [0, adc_motion]

            // Scale from [0, adc_motion] to [speed_min, speed_max]
            speed_target = speed_min + ((adc_absolute * speed_range) / adc_motion);
        }

        // Put the negative sign back in for accleration calculation.
        // Otherwise we would go straight from 100 to -100 instead of
        // smoothly transitioning through zero.
        if (adc_zerocenter < 0) {
            speed_target *= -1;
        }

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

        // Uncomment next line for diagnostic output
        // printf("Speed target %d -- current %d\n", speed_target, speed_current);

        // Output direction to direction_pin
        direction_current = speed_current > 0;
        if (invert_direction) {
            direction_current = !direction_current;
        }
        gpio_set_level(direction_pin, direction_current);

        // Update PWM frequency with newly calculated speed
        if (abs(speed_current) < speed_min) {
            // Deadband. PWM duty cycle zero. PWM frequency irrelevant.
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        }
        else {
            // 16 is 50% duty cycle in 5-bit PWM resolution.
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 16);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

            ledc_set_freq(ledc_timer.speed_mode, ledc_timer.timer_num, abs(speed_current));
        }

        // Wait before repeating, yielding to ESP32 housekeeping chores
        vTaskDelay(pdMS_TO_TICKS(update_period));
    }
}
