#ifndef PWM_H
#define PWM_H

#include "stdint.h"

typedef enum
{
    PWM_OUT_0,
    PWM_OUT_1
} pwm_out_t;

void pwm_init();
void pwm_set_duty_cycle(pwm_out_t pwm_out, uint16_t duty_cycle_percent);

#endif /* PWM_H */
