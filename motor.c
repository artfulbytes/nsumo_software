#include "motor.h"
#include "hw.h"
#include "pwm.h"
#include "stdint.h"

typedef struct
{
    gpio_t cc_1;
    gpio_t cc_2;
    pwm_out_t pwm_out;
} motor_control_t;

static const motor_control_t motor_controls[] =
{
    [MOTORS_LEFT] = {GPIO_MOTORS_LEFT_CC_1, GPIO_MOTORS_LEFT_CC_2, PWM_OUT_0},
    [MOTORS_RIGHT] = {GPIO_MOTORS_RIGHT_CC_1, GPIO_MOTORS_RIGHT_CC_2, PWM_OUT_1},
};

static void motor_set_mode(motor_t motor, motor_mode_t mode)
{
    motor_control_t motor_control = motor_controls[motor];
    switch (mode) {
    case STOP:
        // TODO: Turn off PWM also
        gpio_set_output(motor_control.cc_1, GPIO_LOW);
        gpio_set_output(motor_control.cc_2, GPIO_LOW);
        break;
    case FORWARD:
        gpio_set_output(motor_control.cc_1, GPIO_HIGH);
        gpio_set_output(motor_control.cc_2, GPIO_LOW);
        break;
    case REVERSE:
        gpio_set_output(motor_control.cc_1, GPIO_LOW);
        gpio_set_output(motor_control.cc_2, GPIO_HIGH);
        break;
    }
}

void motor_set_speed(motor_t motor, int16_t speed)
{
    if (speed < -100 || speed > 100) {
        // Only allow speeds between [-100, 100]
        motor_set_mode(motor, STOP);
        pwm_set_duty_cycle(motor_controls[motor].pwm_out, 0);
        return;
    }

    if (speed > 0)
    {
        motor_set_mode(motor, FORWARD);
    }
    else if (speed < 0)
    {
        motor_set_mode(motor, REVERSE);
        speed = -speed;
    }
    else
    {
        motor_set_mode(motor, STOP);
    }

    // Divide by two since motors are 6V max, and at 100% we supply them with the
    // battery voltage which is ~12 V at full charge.
    speed >>= 1;

    pwm_set_duty_cycle(motor_controls[motor].pwm_out, speed);
}

/*
 * There is a lack of timers so we temporarily take over the timer used for PWM
 * to sleep the MCU.
 *
 * NOTE: This function busy waits for 200 ms!
 */
void motor_stop_safely()
{
    motor_set_speed(MOTORS_LEFT, 0);
    motor_set_speed(MOTORS_RIGHT, 0);
    __delay_cycles(200000); //TODO: Maybe use the PWM timer temporarily instead
}

// const and volatile
// Functions should be noun_verb (action or answer question)
// Variables should be noun
void motor_init()
{
    pwm_init();
    gpio_set_direction(motor_controls[MOTORS_LEFT].cc_1, GPIO_OUTPUT);
    gpio_set_direction(motor_controls[MOTORS_LEFT].cc_2, GPIO_OUTPUT);
    gpio_set_direction(motor_controls[MOTORS_RIGHT].cc_1, GPIO_OUTPUT);
    gpio_set_direction(motor_controls[MOTORS_RIGHT].cc_2, GPIO_OUTPUT);
    motor_set_mode(MOTORS_LEFT, STOP);
    motor_set_mode(MOTORS_RIGHT, STOP);
}
