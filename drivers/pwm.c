#include <msp430.h>
#include <stdint.h>

#include "pwm.h"
#include "gpio.h"

/* 100 gives us a nice period because the duty cycle can be used
 * directly to set the timer value (no conversion required) and
 * it gives a frequency of 1 / (100 us) = 10 kHz. */
#define PWM_PERIOD (100)

static uint16_t current_pwm_period = 0;

static void set_period(uint16_t pwm_period)
{
    current_pwm_period = pwm_period;
    TA0CCR0 = pwm_period - 1;
}

/* Set up timer 1 for PWM control
 * Output of capture&compare channel 1 is PWM_OUT_0
 * Output of capture&compare channel 2 is PWM_OUT_1
 */
void pwm_init()
{
    if (PWM_PERIOD != 100) {
        /* Doesn't support other PWM periods at the moment */
        return;
    }
    gpio_set_selection(GPIO_PWM_MOTORS_LEFT, GPIO_SEL_1); // P2.1 select Timer1_A Out1
    gpio_set_direction(GPIO_PWM_MOTORS_LEFT, GPIO_OUTPUT);
    gpio_set_selection(GPIO_PWM_MOTORS_RIGHT, GPIO_SEL_1); // P2.1 select Timer1_A Out2
    gpio_set_direction(GPIO_PWM_MOTORS_RIGHT, GPIO_OUTPUT);

    // TASSEL_X: clock source  (TASSEL_2 = SMCLK)
    // ID_X: input divisor     (ID_1 = divide by 1)
    // MC_X: count mode        (MC_1 = UP mode, counts to TA0CCR0)
    TA0CTL |= TASSEL_2 + MC_1;

    // OUTMOD_7 = Reset/Set mode (positive PWM)
    // Example for pwm 0 signal with CCR1 register
    // -----------------__________
    // <------------T------------>
    // <---------TA0CCR0--------->
    // <----TA0CCR1---->
    TA0CCTL1 |= OUTMOD_7;
    TA0CCTL2 |= OUTMOD_7;

    set_period(PWM_PERIOD);
    pwm_set_duty_cycle(PWM_OUT_0, 0);
    pwm_set_duty_cycle(PWM_OUT_1, 0);
}

// TODO: Look at renaming pwm to with motrs instead...
void pwm_set_duty_cycle(pwm_out_t pwm_out, uint16_t duty_cycle_percent)
{
    if (duty_cycle_percent > 100)
    {
        duty_cycle_percent = 100;
    }
    /* TODO: We might be off by 1 here, when does the interrupt occur at duty_cycle_percent or
     * at duty_cycle_percent - 1?, shouldn't matter in practice though. */
    switch (pwm_out)
    {
    case PWM_OUT_0:
        TA0CCR1 = duty_cycle_percent;
        break;
    case PWM_OUT_1:
        TA0CCR2 = duty_cycle_percent;
        break;
    }
}
