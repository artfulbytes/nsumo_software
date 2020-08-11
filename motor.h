#ifndef __motor
#define __motor

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

#endif //__motor
