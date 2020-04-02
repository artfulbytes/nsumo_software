#include <msp430.h>
#include <stdint.h>

#include "pwm.h"

#define DEFAULT_PWM_PERIOD 8000

static uint16_t current_pwm_period = 0;

void set_pwm_period(uint16_t pwm_period)
{
    current_pwm_period = pwm_period;
    TA1CCR0 = pwm_period-1;
}

static uint16_t get_pwm_period()
{
    return current_pwm_period;
}

/* Set up timer 1 for PWM control
 * Output of capture&compare channel 1 is PWM_OUT1
 * Output of capture&compare channel 2 is PWM_OUT2
 */
void init_pwm()
{
    P2DIR |= BIT1; // P2.1 set as output
    P2SEL |= BIT1; // P2.1 selected Timer1_A Out1
    P2DIR |= BIT4; // P2.4 set as output
    P2SEL |= BIT4; // P2.4 selected Timer1_A Out2

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

    set_pwm_period(DEFAULT_PWM_PERIOD);
    set_duty_cycle(PWM_OUT1, 0);
    set_duty_cycle(PWM_OUT2, 0);
}

void set_duty_cycle(PWM_OUTPUT pwm_out, uint16_t duty_cycle_percent)
{
    uint16_t duty_cycle = 0;

    if (duty_cycle_percent > 0)
    {
        duty_cycle = duty_cycle_percent * (get_pwm_period() / 100) - 1;
    }

    switch (pwm_out)
    {
    case PWM_OUT1:
        TA1CCR1 = duty_cycle;
        break;
    case PWM_OUT2:
        TA1CCR2 |= duty_cycle;
        break;
    }
}
