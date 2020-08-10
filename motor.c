#include "motor.h"
#include "hw.h"

typedef struct
{
    gpio_t cc_1;
    gpio_t cc_2;
} motor_cc_pins_t;

// TODO: It should be possible to map these explicitly
// to the enum (forgot what it's called)
static const motor_cc_pins_t motor_control_pins[] =
{
    [MOTOR_FRONT_LEFT] = {GPIO_MOTOR_FRONT_LEFT_CC_1, GPIO_MOTOR_FRONT_LEFT_CC_2},
    [MOTOR_BACK_LEFT] = {GPIO_MOTOR_BACK_LEFT_CC_1, GPIO_MOTOR_BACK_LEFT_CC_2},
    //{GPIO_MOTOR_FRONT_RIGHT_CC_1, GPIO_MOTOR_FRONT_RIGHT_CC_2},
    //{GPIO_MOTOR_BACK_RIGHT_CC_1, GPIO_MOTOR_BACK_RIGHT_CC_2}
};

void motor_set_mode(motor_t motor, motor_mode_t mode)
{
    motor_cc_pins_t motor_pins = motor_control_pins[motor];
    switch (mode) {
    case STOP:
        gpio_set_output(motor_pins.cc_1, GPIO_LOW);
        gpio_set_output(motor_pins.cc_2, GPIO_LOW);
        break;
    case FORWARD:
        gpio_set_output(motor_pins.cc_1, GPIO_HIGH);
        gpio_set_output(motor_pins.cc_2, GPIO_LOW);
        break;
    case REVERSE:
        gpio_set_output(motor_pins.cc_1, GPIO_LOW);
        gpio_set_output(motor_pins.cc_2, GPIO_HIGH);
        break;
    }
}

// const and volatile
// Functions should be noun_verb (action or answer question)
// Variables should be noun

void motor_init()
{
    gpio_set_direction(motor_control_pins[MOTOR_FRONT_LEFT].cc_1, GPIO_OUTPUT);
    gpio_set_direction(motor_control_pins[MOTOR_FRONT_LEFT].cc_2, GPIO_OUTPUT);
    gpio_set_direction(motor_control_pins[MOTOR_BACK_LEFT].cc_1, GPIO_OUTPUT);
    gpio_set_direction(motor_control_pins[MOTOR_BACK_LEFT].cc_2, GPIO_OUTPUT);
    motor_set_mode(MOTOR_FRONT_LEFT, STOP);
    motor_set_mode(MOTOR_BACK_LEFT, STOP);
    //motor_change_mode(MOTOR_FRONT_RIGHT, STOP);
    //motor_change_mode(MOTOR_BACK_RIGHT, STOP);
}
