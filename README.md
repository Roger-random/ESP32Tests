# Test programs for exploring ESP32

* `template` - Blank template [from Espressif](https://github.com/espressif/esp-idf-template)
* `knob` - Minimalist variant of [ADC sample](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/adc)
Stripped out details about accurate conversion to actual voltage values. This experiment
only concerns about obtaining a range of integer values from an analog reading and skips
the conversion to actual voltage.
* `servo` - Use
[LED Controller (LEDC)](https://docs.espressif.com/projects/esp-idf/en/stable/api-reference/peripherals/ledc.html)
PWM signal to drive hobby RC servo motor position based on `knob` readings.
Code fragments cut and pasted from
[LEDC sample](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/ledc)
to use the low speed LEDC module to generate PWM to control a servo across roughly the
same range we can expect to receive from `knob` ADC. (12 bits or up to 4096 positions.)
* `ESP32_Stepper1` - a nonfunctioning aborted attempt to use the
[ESP-FlexyStepper](https://github.com/pkerspe/ESP-FlexyStepper) library.
* `StepperVelocity` - Reads value of potentiometer using ADC and outputs stepper
control STEP signals using LEDC PWM set to 50% duty cycle but with varying frequency.
* `FreeRTOS_Play` - learning FreeRTOS APIs.
* `MCPWM_DutyCycle` - Interactive MCPWM duty cycle explorer.
* `webserver_play` - learning how to turn an ESP32 into a small web server.
