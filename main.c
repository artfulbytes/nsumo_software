#include <msp430.h>
#include <stdint.h>
#include "pwm.h"

void init_clocks()
{
    BCSCTL1 = CALBC1_1MHZ; // Basic Clock System Control 1 (MCLK?)
    DCOCTL = CALDCO_1MHZ;  // DCO Clock Frequency Control
}

void main(void)
{
    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    volatile unsigned int i; // volatile to prevent optimization

    init_clocks();
    init_pwm();
    _enable_interrupt(); // is this really needed? Try without
    //set_duty_cycle(PWM_OUT1, 50);
    set_duty_cycle(PWM_OUT2, 50);
    uint16_t dim_value = 0;
    uint8_t count_direction = 0;
    while(1)
    {
        if (dim_value >= 100 || dim_value <= 0) {
            count_direction ^= 1;
        }
        dim_value += 5*(count_direction ? 1 : -1);

        set_duty_cycle(PWM_OUT1, dim_value);
        for(i=3000; i>0; i--);     // delay
    }
}


