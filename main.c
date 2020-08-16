#include <msp430.h>
#include <stdint.h>
#include "pwm.h"
#include "hw.h"
#include "motor.h"
#include "adc.h"

// Test program to dim the led up and down using pwm
void test_dimming_led()
{
    volatile unsigned int i;
    uint16_t dim_value = 0;
    uint8_t count_direction = 0;
    for(;;)
    {
        if (dim_value >= 50 || dim_value <= 0) {
            count_direction ^= 1;
        }
        dim_value += 5*(count_direction ? 1 : -1);

        pwm_set_duty_cycle(PWM_OUT_0, dim_value);
        for(i=3000; i>0; i--); // busy loop delay
    }
}

void test_run_motors()
{
    motor_init();
    motor_set_speed(MOTORS_LEFT, 50);
    motor_set_speed(MOTORS_RIGHT, 50);
    for(;;);
}

void test_adc()
{
    adc_init();
    adc_channel_values_t channel_values;
    for(;;)
    {
        adc_read_channels(&channel_values);
    }
}

void main(void)
{
    hw_init();
    pwm_init();

    _enable_interrupt();

    //test_dimming_led();
    //test_run_motors();
    test_adc();
}
