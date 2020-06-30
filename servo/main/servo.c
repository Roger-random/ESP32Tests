#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/ledc.h"

#define NO_OF_SAMPLES   64          //Multisampling

static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2

void app_main(void)
{
    // Set up analog-to-digital converter to read knob position
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_11);

    // Set up LED controller module to generate PWM pulses

    // Step 1: Configure a LEDC timer component. A single timer can service
    //         multiple LED channels.
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_15_BIT, // resolution of PWM duty
        .freq_hz = 50,                        // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);

    // Step 2: Configure LED channel. (References timer set up above.)
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_2,
        .duty       = 0,
        .gpio_num   = 4,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_1
    };
    ledc_channel_config(&ledc_channel);

    uint32_t printskip = 0;
    uint32_t adc_reading = 0;
    uint32_t pwm_duty = 0;
    while (1) {
        adc_reading = 0;
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            // ADC_WIDTH_BIT_12 means no overflow as long as NO_OF_SAMPLES
            // is less than 2^20. (32-12 = 20)
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
        adc_reading /= NO_OF_SAMPLES;

        // Use knob value as LED PWM duty cycle
        pwm_duty = 410 + adc_reading;
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, pwm_duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

        if (printskip < 10)
        {
            printskip++;
        }
        else
        {
            printf("Knob at [%d] PWM duty at [%d]\n", adc_reading, pwm_duty);
            printskip = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
