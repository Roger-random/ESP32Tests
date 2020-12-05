// ESP32 pulse generation test intended to control velocity of a stepper motor.
// Does not track number of steps, and hence unsuitable for position control.

// PWM generation code adapted from Espressif ESP-IDF LEDC PWM example
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/ledc/main/ledc_example_main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

// Only certain combinations of PWM frequency vs. duty cycle are allowed.
// The higher the frequency, the coarser the control over duty cycle.
// At 40MHz only 1-bit duty cycle control is possible: 0% 50%, and 100%.
// At 20MHz, 2-bit, etc.
//
// This project desires a minimum PWM frequency of 8Hz, which imposes
// a lower limit of 7-bits on duty cycle, which in turn imposes an
// maximum PWM frequency of 625kHz.

void app_main(void)
{
    // Configure timer to be used by LEDC peripheral
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

    // Configure LEDC channel to control output pin
    ledc_channel_config_t ledc_channel = {
        .timer_sel  = LEDC_TIMER_0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0, // Start out stopped (0% duty cycle)
        .hpoint     = 0,
        .gpio_num   = 12,
    };
    ledc_channel_config(&ledc_channel);

    // Setup complete, enter infinite loop
    while (1) {
        // On/off blink test.

        // 64 is 50% duty cycle in 7-bit resolution.
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 64);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        // 0%
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
