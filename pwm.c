#include <msp430.h>
#include <stdint.h>

#include "pwm.h"
#include "hw.h"

#define DEFAULT_PWM_PERIOD 8000

static uint16_t current_pwm_period = 0;

static void pwm_set_period(uint16_t pwm_period)
{
    current_pwm_period = pwm_period;
    TA1CCR0 = pwm_period-1;
}

static uint16_t pwm_get_period()
{
    return current_pwm_period;
}

/* Set up timer 1 for PWM control
 * Output of capture&compare channel 1 is PWM_OUT_0
 * Output of capture&compare channel 2 is PWM_OUT_1
 */
void pwm_init()
{
    gpio_set_selection(GPIO_PWM_0, GPIO_SEL_1); // P2.1 select Timer1_A Out1
    gpio_set_direction(GPIO_PWM_0, GPIO_OUTPUT);
    gpio_set_selection(GPIO_PWM_1, GPIO_SEL_1); // P2.1 select Timer1_A Out2
    gpio_set_direction(GPIO_PWM_1, GPIO_OUTPUT);

    // TASSEL_X: clock source  (TASSEL_2 = SMCLK)
    // ID_X: input divisor     (ID_1 = divide by 1)
    // MC_X: count mode        (MC_1 = UP mode, counts to TA1CCR0)
    TA1CTL |= TASSEL_2 + MC_1;

    // OUTMOD_7 = Reset/Set mode (positive PWM)
    // -----------------__________
    // <------------T------------>
    // <---------TA1CCR0--------->
    // <----TA1CCR1---->
    TA1CCTL1 |= OUTMOD_7;
    TA1CCTL2 |= OUTMOD_7;

    pwm_set_period(DEFAULT_PWM_PERIOD);
    pwm_set_duty_cycle(PWM_OUT_0, 0);
    pwm_set_duty_cycle(PWM_OUT_1, 0);
}

void pwm_set_duty_cycle(pwm_out_t pwm_out, uint16_t duty_cycle_percent)
{
    uint16_t duty_cycle = 0;

    if (duty_cycle_percent > 100)
    {
        duty_cycle_percent = 100;
    }

    if (duty_cycle_percent > 0)
    {
        duty_cycle = duty_cycle_percent * (pwm_get_period() / 100) - 1;
    }

    switch (pwm_out)
    {
    case PWM_OUT_0:
        TA1CCR1 = duty_cycle;
        break;
    case PWM_OUT_1:
        TA1CCR2 |= duty_cycle;
        break;
    }
}
