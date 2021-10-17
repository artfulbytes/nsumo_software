#include "test.h"
#include "motor.h"
#include "adc.h"
#include "pwm.h"
#include "gpio.h"
#include "range_sensor.h"
#include "ir_remote.h"
#include "state_machine_ir.h"
#include "time.h"
#include "drivers/led.h"
#include "vl53l0x.h"

void test_dimming_led()
{
    pwm_init();
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
    motor_set_duty_cycle(MOTORS_LEFT, 50);
    motor_set_duty_cycle(MOTORS_RIGHT, 50);
    for(;;);
}

void test_adc()
{
    adc_init();
    adc_channel_values_t channel_values = { 0 };
    for(;;)
    {
        adc_read(&channel_values);
    }
}

typedef struct ranges
{
     uint16_t left;
     uint16_t front_left;
     uint16_t front;
     uint16_t front_right;
     uint16_t right;
} ranges_t;

void test_vl53l0x()
{
    ranges_t ranges;
    bool success = vl53l0x_init();
    while (success) {
        success = vl53l0x_read_range_single(VL53L0X_IDX_FRONT, &ranges.front);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_LEFT, &ranges.left);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_RIGHT, &ranges.right);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_FRONT_LEFT, &ranges.front_left);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_FRONT_RIGHT, &ranges.front_right);
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

void test_state_machine_ir()
{
    state_machine_ir_init();

    bool show_led = true;
    ir_remote_init();
    for(;;) {
        volatile ir_remote_command_t ir_command = ir_remote_get_command();
        if (ir_command != COMMAND_NONE) {
            state_machine_ir_handle_command(ir_command);
        }
        led_set_enable(LED_TEST, show_led);
        show_led = !show_led;
        __delay_cycles(50000);
    }
}
