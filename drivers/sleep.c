#include "sleep.h"
#include <msp430.h>

void sleep_ms(uint32_t ms)
{
    // Cap it at ~2 seconds for now
    if (ms >= 1985) {
        ms = 1985;
    }

    // 1 second = 32768 ticks
    // 1 millisecond ~= 33 ticks
    TA1CCR0 = ms * 33;
    TACCTL0 = CCIE;  // TACCR0 interrupt enabled

    // SLEEP!
}
