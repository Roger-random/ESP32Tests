#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define NO_OF_SAMPLES   64          //Multisampling

static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2

void app_main(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_11);

    uint32_t adc_reading = 0;
    while (1) {
        adc_reading = 0;
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            // ADC_WIDTH_BIT_12 means no overflow as long as NO_OF_SAMPLES
            // is less than 2^20. (32-12 = 20)
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
        adc_reading /= NO_OF_SAMPLES;

        printf("Knob at [%d]\n", adc_reading);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
