#ifndef __motor
#define __motor

typedef enum
{
    MOTOR_FRONT_LEFT,
    MOTOR_BACK_LEFT,
    MOTOR_FRONT_RIGHT,
    MOTOR_BACK_RIGHT
} motor_t;

typedef enum
{
    FORWARD,
    REVERSE,
    STOP
} motor_mode_t;

void motor_init();
void motor_set_mode(motor_t motor, motor_mode_t mode);

#endif //__motor
