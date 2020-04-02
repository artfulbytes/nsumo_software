#ifndef __pwm
#define __pwm

typedef enum
{
    PWM_OUT1,
    PWM_OUT2
} PWM_OUTPUT;

void init_pwm();
void set_duty_cycle(PWM_OUTPUT pwm_out, uint16_t duty_cycle_percent);

#endif __pwm
