#include "test.h"
#include "line_detection.h"
#include "enemy_detection.h"
#include "drive.h"
#include "trace.h"
#include "motor.h"

#ifdef BUILD_MCU
#include "adc.h"
#include "pwm.h"
#include "gpio.h"
#include "ir_remote.h"
#include "state_machine_ir.h"
#include "millis.h"
#include "vl53l0x.h"
#include "led.h"
#include "qre1113.h"
#else // Simulator
#include "microcontroller_c_bindings.h"
#endif

#ifdef BUILD_MCU
void test_dimming_led()
{
    pwm_init();
    volatile unsigned int i;
    uint16_t dim_value = 0;
    uint8_t count_direction = 0;
    for (;;) {
        if (dim_value >= 50 || dim_value <= 0) {
            count_direction ^= 1;
        }
        dim_value += 5 * (count_direction ? 1 : -1);

        pwm_set_duty_cycle(PWM_OUT_0, dim_value);
        for (i = 3000; i > 0; i--)
            ; // busy loop delay
    }
}

void test_run_motors()
{
    motor_init();
    motor_set_duty_cycle(MOTORS_LEFT, 50);
    motor_set_duty_cycle(MOTORS_RIGHT, 50);
    for (;;)
        ;
}

void test_adc()
{
    adc_conf_t conf = { { false } };
    conf.enable[GPIO_PIN_IDX(GPIO_10)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_13)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_14)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_15)] = true;
    adc_init(&conf);
    adc_values_t adc_values = { 0 };

    for (;;) {
        adc_read(adc_values);
    }
}

void test_qre1113()
{
    qre1113_init();
    qre1113_voltages_t voltages = { 0 };
    while (1) {
        qre1113_get_voltages(&voltages);
        TRACE_NOPREFIX("Line sensor front left %d\n"
                       "Line sensor front right %d\n"
                       "Line sensor back left %d\n"
                       "Line sensor back right %d",
                       voltages.front_left, voltages.front_right, voltages.back_left,
                       voltages.back_right);
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
            TRACE_NOPREFIX("%d samples in %d ms", sample_cnt - last_sample_cnt,
                           millis() - last_millis);
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
        success &=
            vl53l0x_read_range_single(VL53L0X_IDX_FRONT_LEFT, &ranges[VL53L0X_IDX_FRONT_LEFT]);
        success &=
            vl53l0x_read_range_single(VL53L0X_IDX_FRONT_RIGHT, &ranges[VL53L0X_IDX_FRONT_RIGHT]);
        TRACE_NOPREFIX("Range sensor left %d\n"
                       "Range sensor right %d\n"
                       "Range sensor front left %d\n"
                       "Range sensor front %d\n"
                       "Range sensor front right %d",
                       ranges[VL53L0X_IDX_LEFT], ranges[VL53L0X_IDX_RIGHT],
                       ranges[VL53L0X_IDX_FRONT_LEFT], ranges[VL53L0X_IDX_FRONT],
                       ranges[VL53L0X_IDX_FRONT_RIGHT]);
    }
}

void test_enemy_detection_led()
{
    led_init();
    enemy_detection_init();
    while (1) {
        enemy_detection_t detection = enemy_detection_get();
        if (detection.position != ENEMY_POS_NONE) {
            TRACE_NOPREFIX("%s %s", enemy_pos_str(detection.position),
                           enemy_range_str(detection.range));
            led_set_enable(LED_TEST, true);
        } else {
            led_set_enable(LED_TEST, false);
        }
    }
}

#if 0
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
#endif

void test_gpio_input()
{
    led_init();
    const gpio_config_t gpio_config = { .gpio = GPIO_RANGE_SENSOR_FRONT_INT,
                                        .dir = GPIO_INPUT,
                                        .out = GPIO_LOW,
                                        .resistor = RESISTOR_ENABLED,
                                        .selection = GPIO_SEL_GPIO };

    gpio_configure(&gpio_config);
    while (1) {
        led_set_enable(LED_TEST, gpio_get_input(GPIO_RANGE_SENSOR_FRONT_INT));
    }
}

void test_vl53l0x_multiple()
{
    vl53l0x_ranges_t ranges;
    bool success = vl53l0x_init();
    bool fresh_values = false;
    while (success) {
        success = vl53l0x_read_range_multiple(ranges, &fresh_values);
        TRACE_NOPREFIX("left   %d"
                       "right  %d"
                       "fleft  %d"
                       "front  %d"
                       "fright %d",
                       ranges[VL53L0X_IDX_LEFT], ranges[VL53L0X_IDX_RIGHT],
                       ranges[VL53L0X_IDX_FRONT_LEFT], ranges[VL53L0X_IDX_FRONT],
                       ranges[VL53L0X_IDX_FRONT_RIGHT]);
    }
}

void test_state_machine_ir()
{
    state_machine_ir_init();

    bool show_led = true;
    ir_remote_init();
    while (1) {
        ir_key_t key = ir_remote_get_key();
        if (key != IR_KEY_NONE) {
            state_machine_ir_handle_key(key);
        }
        led_set_enable(LED_TEST, show_led);
        show_led = !show_led;
        __delay_cycles(50000);
    }
}

void test_ir_receiver()
{
    ir_remote_init();
    volatile uint16_t keypresses = 0;

    for (;;) {
        if (ir_remote_get_key() != IR_KEY_NONE) {
            keypresses++;
        }
    }
}

void test_drives_remote()
{
    ir_remote_init();
    drive_init();
    drive_speed_t speed = DRIVE_SPEED_SLOW;
    drive_t drive = DRIVE_ARCTURN_SHARP_LEFT;
    while (1) {
        const ir_key_t key = ir_remote_get_key();
        if (key != IR_KEY_NONE) {
            TRACE_NOPREFIX("Ir key %d", key);
        } else {
            __delay_cycles(50000);
            continue;
        }
        switch (key) {
        case IR_KEY_0:
            speed = DRIVE_SPEED_SLOW;
            break;
        case IR_KEY_1:
            speed = DRIVE_SPEED_MEDIUM;
            break;
        case IR_KEY_2:
            speed = DRIVE_SPEED_FAST;
            break;
        case IR_KEY_3:
            speed = DRIVE_SPEED_FASTEST;
            break;
        case IR_KEY_4:
            drive = DRIVE_ARCTURN_SHARP_LEFT;
            break;
        case IR_KEY_5:
            drive = DRIVE_ARCTURN_MID_LEFT;
            break;
        case IR_KEY_6:
            drive = DRIVE_ARCTURN_WIDE_LEFT;
            break;
        case IR_KEY_7:
        case IR_KEY_8:
        case IR_KEY_9:
        case IR_KEY_LEFT:
        case IR_KEY_RIGHT:
        case IR_KEY_STAR:
        case IR_KEY_HASH:
        case IR_KEY_UP:
        case IR_KEY_DOWN:
        case IR_KEY_OK:
        case IR_KEY_NONE:
            drive_stop();
            continue;
            break;
        }
        drive_set(drive, false, speed);
        __delay_cycles(50000);
    }
}
#endif // BUILD_MCU

void test_line_detection()
{
    line_detection_init();
    while (1) {
        line_detection_t line_detection = line_detection_get();
        TRACE_NOPREFIX("%s", line_detection_str(line_detection));
    }
}

void test_drive_and_line_detect()
{
    line_detection_init();
    drive_init();

    const drive_speed_t speed = DRIVE_SPEED_FASTEST;
    drive_t current_drive = DRIVE_FORWARD;
    drive_set(current_drive, false, speed);
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
        case LINE_DETECTION_DIAGONAL_LEFT:
        case LINE_DETECTION_DIAGONAL_RIGHT:
            break;
        }

        if (new_drive != current_drive) {
            drive_set(new_drive, false, speed);
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
        const enemy_detection_t detection = enemy_detection_get();
        new_rotate = (detection.position != ENEMY_POS_FRONT);

        if (new_rotate != rotate) {
            if (new_rotate) {
                drive_set(DRIVE_ROTATE_LEFT, false, DRIVE_SPEED_FASTEST);
            } else {
                drive_stop();
            }
            rotate = new_rotate;
        }
    }
}

void test_enemy_detection_print()
{
    enemy_detection_init();
    while (1) {
        enemy_detection_t detection = enemy_detection_get();
        TRACE_NOPREFIX("%s %s", enemy_pos_str(detection.position),
                       enemy_range_str(detection.range));
    }
}

void test_drive_duty_cycles()
{
    drive_init();
    drive_set(DRIVE_ARCTURN_WIDE_LEFT, false, DRIVE_SPEED_FASTEST);
    while (1) {
        millis();
    }
}
