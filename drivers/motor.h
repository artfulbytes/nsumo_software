#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

typedef enum
{
    MOTORS_LEFT,
    MOTORS_RIGHT
} motors_t;

void motor_init(void);
void motor_set_duty_cycle(motors_t motors, int16_t duty_cycle);
void motor_stop_safely(void);

#endif /* MOTOR_H */
