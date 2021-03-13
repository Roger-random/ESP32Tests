# Interactive MCPWM Duty Cycle Explorer

This is a tool for exploring a motor's response to different MCPWM duty cycles.
Emits six DRV8833 control signals simultaneously so up to six motors could
be tested in parallel to see how much variation there is between motors in the
same batch. See src/duty_cycle.h for GPIO pin assignments.

Interactive control is from an analog joystick connected to ESP32 input pins
with two axes connected to ESP32 ADC and a digital button. The button may
require an external pull-up resistor if the assigned GPIO pin lacks built-in
pull-up resistors. See src/joy_adc.h for GPIO pin assignments.

Joystick is treated as a directional pad here so all actions are on/off with
no analog partial progression. Movement along one axis adjusts the duty
cycle incremental rate. 10%, 1% or 0.1%. Movement along the other axis
adjusts the duty cycle up or down by the incremental rate. The button toggles
between the active duty cycle and 0 duty cycle to turn off motor.

This program is only useful when running with serial monitor active. The
currently applicable increment rate and the currently active duty cycle
are printed to the serial port.
