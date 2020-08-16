#ifndef __motor
#define __motor

#include <stdint.h>

typedef enum
{
    MOTORS_LEFT,
    MOTORS_RIGHT,
} motor_t;

typedef enum
{
    FORWARD,
    REVERSE,
    STOP
} motor_mode_t;

void motor_init();
void motor_set_speed(motor_t motor, int16_t speed);

#endif //__motor
