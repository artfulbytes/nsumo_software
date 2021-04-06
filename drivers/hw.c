#include "hw.h"
#include "gpio.h"
#include "uart.h"
#include <msp430.h>

static void hw_init_clocks()
{
    /* SMCLK = 1MHz */
    BCSCTL1 = CALBC1_1MHZ; // Basic Clock System Control 1
    DCOCTL = CALDCO_1MHZ;  // DCO Clock Frequency Control

    /* ACLK with VLOCLK */
    BCSCTL3 = LFXT1S_2;
}

static void hw_stop_watchdog()
{
    WDTCTL = WDTPW | WDTHOLD;
}

static void hw_setup_watchdog_interrupt()
{
    /* Since we don't need the watchdog for reset, we use it to
     * count the time elapsed instead */
    WDTCTL = WDT_MDLY_0_5;
    IE1 |= WDTIE;
}

static void hw_enable_global_interrupts()
{
    _enable_interrupts();
}

void hw_init()
{
    hw_stop_watchdog();
    hw_setup_watchdog_interrupt();
    hw_init_clocks();
    gpio_init();
    uart_init();
    hw_enable_global_interrupts();
}


