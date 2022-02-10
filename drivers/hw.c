#include "hw.h"
#include "gpio.h"
#include "uart.h"
#include <msp430.h>

static void init_clocks()
{
    /* Sanity check calibration data not erased */
    if (CALBC1_1MHZ == 0xFF || CALBC1_16MHZ == 0xFF) {
        while (1)
            ;
    }

    /* SMCLK = 16MHz */
    BCSCTL1 = CALBC1_16MHZ; // Basic Clock System Control 1
    DCOCTL = CALDCO_16MHZ; // DCO Clock Frequency Control

    /* ACLK with VLOCLK */
    BCSCTL3 = LFXT1S_2;
}

static void stop_watchdog() { WDTCTL = WDTPW | WDTHOLD; }

// 16 MHZ / 32768 = ~2000 Hz
#define WDT_MDLY_0_5_16MHZ (WDTPW + WDTTMSEL + WDTCNTCL + WDTIS0)

static void setup_watchdog_interrupt()
{
    /* Since we don't need the watchdog for reset, we use it to
     * count the time elapsed instead */
    WDTCTL = WDT_MDLY_0_5_16MHZ;
    IE1 |= WDTIE;
}

static void enable_global_interrupts() { _enable_interrupts(); }

void hw_init()
{
    stop_watchdog();
    setup_watchdog_interrupt();
    init_clocks();
    gpio_init();
    enable_global_interrupts();
}
