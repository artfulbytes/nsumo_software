#include <msp430.h>
#include <stdint.h>

#include "pwm.h"
#include "gpio.h"

#define SMCLK_MHZ (16)
#define TIMER_DIVISOR (8)
#define TIMER_COUNT_FREQ_MHZ (SMCLK_MHZ / TIMER_DIVISOR)
#define TIMER_COUNT_FREQ_KHZ (TIMER_COUNT_FREQ_MHZ * 1000)
/* 10 KHz makes it easy and even to convert duty cycle to timer value.
 * This also works well with the motors. ~10-20 KHz seems to be a typical
 * range according to various sources. */
#define PWM_PERIOD_KHZ (10)
#define TA0CCRO_PERIOD_VALUE (TIMER_COUNT_FREQ_KHZ / PWM_PERIOD_KHZ)
#define TA0CCR_FACTOR (TA0CCRO_PERIOD_VALUE / DUTY_CYCLE_MAX)
#define DUTY_CYCLE_MAX (100)
#define DUTY_CYCLE_TO_TA0CCR(duty_cycle) (duty_cycle * TA0CCR_FACTOR)

static void set_pwm_period() { TA0CCR0 = TA0CCRO_PERIOD_VALUE - 1; }

/* Set up timer 1 for PWM control
 * Output of capture&compare channel 1 is PWM_OUT_0
 * Output of capture&compare channel 2 is PWM_OUT_1
 */
void pwm_init()
{
    gpio_set_selection(GPIO_PWM_MOTORS_LEFT, GPIO_SEL_1); // P2.1 select Timer1_A Out1
    gpio_set_direction(GPIO_PWM_MOTORS_LEFT, GPIO_OUTPUT);
    gpio_set_selection(GPIO_PWM_MOTORS_RIGHT, GPIO_SEL_1); // P2.1 select Timer1_A Out2
    gpio_set_direction(GPIO_PWM_MOTORS_RIGHT, GPIO_OUTPUT);

    // TASSEL_X: clock source  (TASSEL_2 = SMCLK = 16 MHz)
    // ID_X: input divisor     (ID_3 = divide by 8)
    // MC_X: count mode        (MC_1 = UP mode, counts to TA0CCR0)
    TA0CTL |= TASSEL_2 + ID_3 + MC_1;

    // OUTMOD_7 = Reset/Set mode (positive PWM)
    // Example for pwm 0 signal with CCR1 register
    // -----------------__________
    // <------------T------------>
    // <---------TA0CCR0--------->
    // <----TA0CCR1---->
    TA0CCTL1 |= OUTMOD_7;
    TA0CCTL2 |= OUTMOD_7;

    set_pwm_period();
    pwm_set_duty_cycle(PWM_OUT_0, 0);
    pwm_set_duty_cycle(PWM_OUT_1, 0);
}

// TODO: Look at renaming pwm to with motrs instead...
void pwm_set_duty_cycle(pwm_out_t pwm_out, uint16_t duty_cycle_percent)
{
    if (duty_cycle_percent > 100) {
        duty_cycle_percent = 100;
    }
    /* TODO: We might be off by 1 here, when does the interrupt occur at duty_cycle_percent or
     * at duty_cycle_percent - 1?, shouldn't matter in practice though. */
    switch (pwm_out) {
    case PWM_OUT_0:
        TA0CCR1 = DUTY_CYCLE_TO_TA0CCR(duty_cycle_percent);
        break;
    case PWM_OUT_1:
        TA0CCR2 = DUTY_CYCLE_TO_TA0CCR(duty_cycle_percent);
        break;
    }
}
