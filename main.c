#include <msp430.h>
#include <stdint.h>
#include "pwm.h"
#include "hw.h"
#include "motor.h"
#include "adc.h"
#include "sensors.h"
#include "ir_remote.h"
#include "state_machine.h"

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
    motor_set_speed(MOTORS_LEFT, 40);
    motor_set_speed(MOTORS_RIGHT, 40);
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

void test_sensors()
{
    sensors_init();
    sensor_distances_t distances;

    for(;;)
    {
        sensors_get_distances(&distances);
    }
}

void test_ir_receiver()
{
    ir_remote_init();
    volatile uint16_t keypresses = 0;

    for(;;)
    {
        if (ir_remote_get_command() != COMMAND_NONE) {
            keypresses++;
        }
    }
}

void test_state_machine()
{
    state_machine_init();
    ir_remote_init();
    motor_init();
    for(;;) {
        volatile ir_remote_command_t ir_command = ir_remote_get_command();
        if (ir_command != COMMAND_NONE) {
            state_machine_handle_ir_command(ir_command);
        }
        __delay_cycles(50000);
    }
}

void main(void)
{
    hw_init();

    _enable_interrupts();

    //test_dimming_led();
    //test_run_motors();
    //test_adc();
    //test_sensors();
    //test_ir_receiver();
    test_state_machine();
}
