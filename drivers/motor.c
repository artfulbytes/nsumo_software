#include "motor.h"
#include "gpio.h"
#include "pwm.h"
#include "stdint.h"

typedef struct
{
    gpio_t cc_1;
    gpio_t cc_2;
    pwm_out_t pwm_out;
} motor_control_t;

typedef enum
{
    MOTOR_MODE_STOP,
    MOTOR_MODE_FORWARD,
    MOTOR_MODE_REVERSE
} motor_mode_t;

static const motor_control_t motor_controls[] = {
    [MOTORS_LEFT] = { GPIO_MOTORS_LEFT_CC_1, GPIO_MOTORS_LEFT_CC_2, PWM_OUT_0 },
    [MOTORS_RIGHT] = { GPIO_MOTORS_RIGHT_CC_1, GPIO_MOTORS_RIGHT_CC_2, PWM_OUT_1 },
};

static void set_mode(motors_t motor, motor_mode_t mode)
{
    motor_control_t motor_control = motor_controls[motor];
    switch (mode) {
    case MOTOR_MODE_STOP:
        /* TODO: Turn off PWM also */
        gpio_set_output(motor_control.cc_1, GPIO_LOW);
        gpio_set_output(motor_control.cc_2, GPIO_LOW);
        break;
    case MOTOR_MODE_FORWARD:
        gpio_set_output(motor_control.cc_1, GPIO_HIGH);
        gpio_set_output(motor_control.cc_2, GPIO_LOW);
        break;
    case MOTOR_MODE_REVERSE:
        gpio_set_output(motor_control.cc_1, GPIO_LOW);
        gpio_set_output(motor_control.cc_2, GPIO_HIGH);
        break;
    }
}

static uint16_t get_scaled_duty_cycle(uint16_t unscaled_duty_cycle)
{
    /* Multiply with 3/4 since motors are 6V max, and when batteries are 100%
     * charged they give ~8V. */
    return (unscaled_duty_cycle / 4) * 3;
}

void motor_set_duty_cycle(motors_t motors, int16_t duty_cycle)
{
    if (duty_cycle < -100 || 100 < duty_cycle) {
        /* Only allow duty cycle between [-100, 100] */
        set_mode(motors, MOTOR_MODE_STOP);
        pwm_set_duty_cycle(motor_controls[motors].pwm_out, 0);
        /* TODO: Trace warning */
        return;
    }

    if (duty_cycle > 0) {
        set_mode(motors, MOTOR_MODE_FORWARD);
    } else if (duty_cycle < 0) {
        set_mode(motors, MOTOR_MODE_REVERSE);
        /* TODO: Why do we need to reverse the duty cycle, shouldn't be needed!!!???*/
        duty_cycle = -duty_cycle;
    } else {
        set_mode(motors, MOTOR_MODE_STOP);
    }

    duty_cycle = get_scaled_duty_cycle(duty_cycle);
    pwm_set_duty_cycle(motor_controls[motors].pwm_out, duty_cycle);
}

/*
 * There is a lack of timers so we temporarily take over the timer used for PWM
 * to sleep the MCU.
 *
 * NOTE: This function busy waits for 200 ms!
 */
void motor_stop_safely()
{
    motor_set_duty_cycle(MOTORS_LEFT, 0);
    motor_set_duty_cycle(MOTORS_RIGHT, 0);
    __delay_cycles(200000); /* TODO: Maybe use the PWM timer temporarily instead */
}

void motor_init()
{
    pwm_init();
    gpio_set_direction(motor_controls[MOTORS_LEFT].cc_1, GPIO_OUTPUT);
    gpio_set_direction(motor_controls[MOTORS_LEFT].cc_2, GPIO_OUTPUT);
    gpio_set_direction(motor_controls[MOTORS_RIGHT].cc_1, GPIO_OUTPUT);
    gpio_set_direction(motor_controls[MOTORS_RIGHT].cc_2, GPIO_OUTPUT);
    set_mode(MOTORS_LEFT, MOTOR_MODE_STOP);
    set_mode(MOTORS_RIGHT, MOTOR_MODE_STOP);
}
