#include "hw.h"
#include "gpio.h"

#include <msp430.h>

static void hw_init_clocks()
{
    BCSCTL1 = CALBC1_1MHZ; // Basic Clock System Control 1 (MCLK?)
    DCOCTL = CALDCO_1MHZ;  // DCO Clock Frequency Control
}

static void hw_stop_watchdog_timer()
{
    // Stop this or the MSP430 will keep rebooting
    WDTCTL = WDTPW | WDTHOLD;
}

void hw_init()
{
    hw_stop_watchdog_timer();
    hw_init_clocks();
    gpio_init();
    _enable_interrupts();
}
