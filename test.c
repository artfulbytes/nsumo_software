#include "test.h"
#include "motor.h"
#include "adc.h"
#include "pwm.h"
#include "gpio.h"
#include "range_sensor.h"
#include "ir_remote.h"
#include "state_machine_ir.h"
#include "time.h"
#include "vl53l0x.h"
#include "led.h"
#include "qre1113.h"
#include "trace.h"
#include "line_detection.h"
#include "opponent_detection.h"

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
    adc_conf_t conf = { {false} };
    conf.enable[GPIO_PIN_IDX(GPIO_10)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_13)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_14)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_15)] = true;
    adc_init(&conf);
    adc_values_t adc_values = { 0 };

    for(;;)
    {
        adc_read(adc_values);
    }
}

void test_qre1113()
{
    led_init();
    qre1113_init();
    qre1113_voltages_t voltages = {0};
    while (1) {
        qre1113_get_voltages(&voltages);
        trace("Line sensor front left %d\n"
              "Line sensor front right %d\n"
              "Line sensor back left %d\n"
              "Line sensor back right %d\n",
              voltages.front_left, voltages.front_right,
              voltages.back_left, voltages.back_right);
        __delay_cycles(500000);
    }
}

void test_line_detection()
{
    line_detection_init();
    while (1) {
        line_detection_t line_detection = line_detection_get();
        trace("%s\n", line_detection_enum_to_str(line_detection));
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
        trace("Range sensor left %d\n"
              "Range sensor right %d\n"
              "Range sensor front left %d\n"
              "Range sensor front %d\n"
              "Range sensor front right %d\n",
              ranges.left, ranges.right,
              ranges.front_left, ranges.front,
              ranges.front_right);
        __delay_cycles(500000);
    }
}

void test_enemy_detection()
{
    led_init();
    enemy_detection_init();
    while (1) {
        if (
            (enemy_detection_get() & ENEMY_DETECTION_RIGHT) ||
            (enemy_detection_get() & ENEMY_DETECTION_FRONT) ||
            (enemy_detection_get() & ENEMY_DETECTION_LEFT)) {
            led_set_enable(LED_TEST, true);
        } else {
            led_set_enable(LED_TEST, false);
        }
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
