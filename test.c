#include "test.h"
#include "line_detection.h"
#include "enemy_detection.h"
#include "drive.h"
#include "trace.h"

#if BUILD_MCU
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
        __delay_cycles(50000);
    }
}

void test_qre1113_time()
{
    qre1113_init();
    uint32_t last_sample_cnt = adc_total_sample_cnt();
    uint32_t last_millis = millis();
    while (1) {
        uint32_t sample_cnt = adc_total_sample_cnt();
        if (sample_cnt - last_sample_cnt >= 100) {
            trace("%d samples in %d ms\n", sample_cnt - last_sample_cnt, millis() - last_millis);
            last_sample_cnt = sample_cnt;
            last_millis = millis();
        }
        __delay_cycles(50000);
    }
}

void test_vl53l0x()
{
    vl53l0x_ranges_t ranges;
    bool success = vl53l0x_init();
    while (success) {
        success = vl53l0x_read_range_single(VL53L0X_IDX_FRONT, &ranges[VL53L0X_IDX_FRONT]);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_LEFT, &ranges[VL53L0X_IDX_LEFT]);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_RIGHT, &ranges[VL53L0X_IDX_RIGHT]);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_FRONT_LEFT, &ranges[VL53L0X_IDX_FRONT_LEFT]);
        success &= vl53l0x_read_range_single(VL53L0X_IDX_FRONT_RIGHT, &ranges[VL53L0X_IDX_FRONT_RIGHT]);
        trace("Range sensor left %d\n"
              "Range sensor right %d\n"
              "Range sensor front left %d\n"
              "Range sensor front %d\n"
              "Range sensor front right %d\n",
              ranges[VL53L0X_IDX_LEFT], ranges[VL53L0X_IDX_RIGHT],
              ranges[VL53L0X_IDX_FRONT_LEFT], ranges[VL53L0X_IDX_FRONT],
              ranges[VL53L0X_IDX_FRONT_RIGHT]);
    }
}

void test_vl53l0x_multiple()
{
    vl53l0x_ranges_t ranges;
    bool success = vl53l0x_init();
    while (success) {
        success = vl53l0x_read_range_multiple(ranges);
        trace("left   %d"
              "right  %d"
              "fleft  %d"
              "front  %d"
              "fright %d\n",
              ranges[VL53L0X_IDX_LEFT], ranges[VL53L0X_IDX_RIGHT],
              ranges[VL53L0X_IDX_FRONT_LEFT], ranges[VL53L0X_IDX_FRONT],
              ranges[VL53L0X_IDX_FRONT_RIGHT]);
    }
}

void test_vl53l0x_multiple_time()
{
    vl53l0x_ranges_t ranges;
    bool success = vl53l0x_init();
    int measure_cnt = 0;
    uint32_t last_millis = millis();
    while (success) {
        success = vl53l0x_read_range_multiple(ranges);
        measure_cnt++;
        if (!(measure_cnt % 100)) {
            trace("100 measures in %ld ms\n", millis()-last_millis);
            last_millis = millis();
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

void test_gpio_input()
{
    led_init();
    const gpio_config_t gpio_config = {
        .gpio = GPIO_RANGE_SENSOR_FRONT_INT,
        .dir = GPIO_INPUT,
        .out = GPIO_LOW,
        .resistor = RESISTOR_ENABLED,
        .selection = GPIO_SEL_GPIO
    };

    gpio_configure(&gpio_config);
    while (1) {
        led_set_enable(LED_TEST, gpio_get_input(GPIO_RANGE_SENSOR_FRONT_INT));
    }
}

#endif

void test_line_detection()
{
    line_detection_init();
    while (1) {
        line_detection_t line_detection = line_detection_get();
        trace("%s\n", line_detection_enum_to_str(line_detection));
    }
}

void test_drive_and_line_detect()
{
    line_detection_init();
    drive_init();

    const drive_speed_t speed = DRIVE_SPEED_FASTEST;
    drive_t current_drive = DRIVE_FORWARD;
    drive_set(current_drive, speed);
    while (1) {
        drive_t new_drive = current_drive;
        switch (line_detection_get()) {
        case LINE_DETECTION_FRONT:
        case LINE_DETECTION_FRONT_LEFT:
        case LINE_DETECTION_FRONT_RIGHT:
            new_drive = DRIVE_REVERSE;
            break;
        case LINE_DETECTION_BACK:
        case LINE_DETECTION_BACK_LEFT:
        case LINE_DETECTION_BACK_RIGHT:
            new_drive = DRIVE_FORWARD;
            break;
        case LINE_DETECTION_NONE:
        case LINE_DETECTION_LEFT:
        case LINE_DETECTION_RIGHT:
            break;
        }

        if (new_drive != current_drive) {
            drive_set(new_drive, speed);
            current_drive = new_drive;
        }
    }
}

void test_rotate_and_enemy_detect()
{
    enemy_detection_init();
    drive_init();

    bool rotate = false;
    bool new_rotate = false;
    while (1) {
        new_rotate = !(enemy_detection_get() & (ENEMY_DETECTION_FRONT));// | ENEMY_DETECTION_FRONT_RIGHT | ENEMY_DETECTION_FRONT_LEFT));

        if (new_rotate != rotate) {
            if (new_rotate) {
                drive_set(DRIVE_ROTATE_LEFT, DRIVE_SPEED_FASTEST);
            } else {
                drive_stop();
            }
            rotate = new_rotate;
        }
    }
}
