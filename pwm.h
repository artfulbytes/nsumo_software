#ifndef __pwm
#define __pwm

typedef enum
{
    PWM_OUT_0,
    PWM_OUT_1
} PWM_OUTPUT;

void pwm_init();
void pwm_set_duty_cycle(PWM_OUTPUT pwm_out, uint16_t duty_cycle_percent);

#endif //__pwm
