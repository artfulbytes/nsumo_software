#include "time.h"
#include <msp430.h>

/* Timer description
 *
 * TACTL (Timer control)
 *  TASSEL_X: Selects the clock source (TACLK, ACLK (32kHz), SMCLK (high freq ~1MHz), INCLK)
 *  ID_X: Divider (1/2/4/8)
 *  MC_X: Count mode (Stop, Up (count from 0 to CCR0), Continuous up (count from 0 to max), up/down)
 *  TACLR:  (Clears TAR register AND divider! AND count direction!)
 *  TAIE: Enable TAIFG interrupt request
 *  TAIFG: Check if interrupt pending
 *
 * TAR
 *  16-bit count value of timer
 *
 *
 * TACCTL0, TACCTL1, TACCTL2
 *  Capture and compare control
 *  CM_X: Set specific capture mode
 *  CAP: Set which mode (Compare or Capture mode), compare is default
 *  OUTMODE_X: Set output mode (Several modes doesn't work for TACCR0)
 *  CCIE: Enable Interrupt
 *  CCIFG: Check if interrupt pending
 *
 *
 * TA1CCR0, TA1CCR1, TA2CCR2
 *   Capture and compare period
 *   If compare mode: the TAR value to react on
 *   If capture mode: storage for capture TAR value
 *
 *
 * TAIFG
 *  Sets when timer rolls over to zero
 * CCIFG
 *  Sets when timer reaches last value
 *
 *
 * How to start the timer?
 *  Write MC_X > 0 and set an active clock source
 *
 *
 * COMPARE MODE CAP
 *  I think it's set when CM_0
 *  When reaching the value in TACCRX the CCIFG is set
 *
 *  Do I need output mode if I'm just going to use interrupt?
 *  -
 */

void time_init()
{
    // ACLK and count to CCR0
    TA1CTL |= TASSEL_1 + MC_1;
    TA1CCR0 = 0;

    TACTL = TASSEL_2 + MC_1; // SMCLK, up mode
    TACCR0 = 65000; // interrupt after 65ms
}

volatile uint32_t watchdog_interrupt_cnt = 0;
/* This is how GCC interrupts are declared (#pragma vector=WDT_VECTOR is for
 * TI compiler) */
void watchdog_timer(void) __attribute__ ((interrupt (WDT_VECTOR)));
void watchdog_timer(void)
{
    watchdog_interrupt_cnt++;
}

uint32_t time_ms()
{
    /* Disable interrupts while retrieving the counter */
    IE1 &= ~WDTIE;
    /* Divide by two because the watchdog timer triggers every 0.5 ms */
    uint32_t ms = watchdog_interrupt_cnt >> 1;
    IE1 |= WDTIE;
    return ms;
}
