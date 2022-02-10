#include <msp430.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "printf.h"

static bool initialized = false;

#define UCA0BR0_9600HZ_AT_16MHZ (1666 & 0xFF)
#define UCA0BR1_9600HZ_AT_16MHZ (1666 >> 8)

bool uart_init(void)
{
    if (initialized) {
        return false;
    }
    /* USCI must be reset before configuration */
    if (UCA0CTL1 & UCSWRST) {
        /* Configure SMCLK as clock source */
        UCA0CTL1 |= UCSSEL_2;
        /* Configure baud rate to 9600 as it's the maximum baud rate the Launchpad allows.
         * Values are picked from the table in the family user guide (SLAU144).
         * PRESCALER = UCAxBR0 + (UCAxBR1 * 256)
         * 1 MHZ / PRESCALER ~≃ 9600
         * 8-bit, no parity bit and one stop bit. */
        UCA0BR0 = UCA0BR0_9600HZ_AT_16MHZ;
        UCA0BR1 = UCA0BR1_9600HZ_AT_16MHZ;
        UCA0MCTL = UCBRS0;

        /* Reset the USCI */
        UCA0CTL1 &= ~UCSWRST;
    }
    initialized = true;
    return true;
}

/* This is the internal function used by mpaland/printf (see external/printf) */
/* TODO: This is slow! Use buffering and interrupts? */
void _putchar(char character)
{
    /* Wait for the transfer buffer */
    while (!(IFG2 & UCA0TXIFG))
        ;

    /* Transmit the character */
    UCA0TXBUF = character;

    /* If we get a line-feed, add a carriage return to make new line work
     * properly on the other end. */
    if (character == '\n') {
        while (!(IFG2 & UCA0TXIFG))
            ;
        UCA0TXBUF = '\r';
    }
}

char uart_getchar()
{
    char chr = 0;

    /* Check if we have received anything */
    if (IFG2 & UCA0RXIFG) {
        chr = UCA0RXBUF;
    }

    return chr;
}
