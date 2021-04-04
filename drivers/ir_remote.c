#include <msp430.h>
#include <stdint.h>
#include "ir_remote.h"

// NEC protocol
#define T_AGC_USEC 9000
#define T_AFTER_AGC_USEC 4500
#define T_LOGICAL_ONE_USEC 2250
#define T_LOGICAL_ZERO_USEC 1125
#define T_ERROR_MARGIN_USEC 250
#define T_MAX_USEC 65000
// 110 ms according to spec, but get ~108600 uS when measuring
#define T_REPEATING_USEC 108600
#define T_REPEATING_AFTER_AGC_USEC 2250

// TODO: Larger than it has to be probably
#define MSG_BUFFER_SIZE 20

typedef enum {
    STATE_INACTIVE,
    STATE_AGC,
    STATE_AFTER_AGC,
    STATE_RECEIVING_BITS,
    STATE_REPEATING
} ir_state_t;

static volatile uint32_t msg_buffer[MSG_BUFFER_SIZE] = {0};
static volatile uint16_t msg_buffer_head = 0;
static volatile unsigned int msg_buffer_tail = 0;

// TODO: Do they need to be volatile?
static volatile unsigned int bitcounter = 0;
static volatile uint32_t message = 0;
static volatile uint32_t time_total = 0;
static volatile uint32_t time_passed = 0;
static volatile uint32_t time_last_interrupt = 0;
static volatile uint16_t repeating = 0;
static volatile ir_state_t ir_state = STATE_INACTIVE;

// TODO: Can and should use 16-bit here instead
static ir_remote_command_t msg_to_cmd(uint32_t msg)
{
    switch(msg)
    {
    case 16750695: return COMMAND_0;
    case 16753245: return COMMAND_1;
    case 16736925: return COMMAND_2;
    case 16769565: return COMMAND_3;
    case 16720605: return COMMAND_4;
    case 16712445: return COMMAND_5;
    case 16761405: return COMMAND_6;
    case 16769055: return COMMAND_7;
    case 16754775: return COMMAND_8;
    case 16748655: return COMMAND_9;
    case 16738455: return COMMAND_STAR;
    case 16756815: return COMMAND_HASH;
    case 16718055: return COMMAND_UP;
    case 16730805: return COMMAND_DOWN;
    case 16716015: return COMMAND_LEFT;
    case 16734885: return COMMAND_RIGHT;
    case 16726215: return COMMAND_OK;
    }
    return COMMAND_NONE;
}

// Add any message (even invalid ones)
// We check validity in get function.
static inline void ir_remote_msg_buffer_add(uint32_t msg)
{
    msg_buffer[msg_buffer_head++] = msg;
    if (msg_buffer_head >= MSG_BUFFER_SIZE) {
        // Avoid slow modulo on MSP430 in interrupt context (no native support)
        msg_buffer_head = 0;
    }
    if (msg_buffer_head == msg_buffer_tail) {
        msg_buffer_tail++;
        if (msg_buffer_tail >= MSG_BUFFER_SIZE) {
            msg_buffer_tail = 0;
        }
    }
}

static inline void ir_remote_reset()
{
    // TODO: Is this how to correctly stop the timer?
    //       We might just need TACLR, since it also clears MC
    TACTL = MC_0 + TACLR;
    ir_state = STATE_INACTIVE;
    bitcounter = 0;
    time_total = 0;
    time_last_interrupt = 0;
    time_passed = 0;
    repeating = 0;
    message = 0;
    P1IES |= BIT5; // falling edge
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (time_total == T_MAX_USEC) {
        // Timeout -> reset and stop
        ir_remote_reset();
    } else {
        time_total += T_MAX_USEC;
    }
}

// TODO: What if both interrupts happen simultaneously?
// TODO: Verify with inverted bits??
// TODO: Just get flank and then do side processing somewhere else?
// TODO: Measure time from start to finish inside this function
//       (Quick test, 20-120 uS, so below one millisecond, negligble I think)
// TODO: How do I get the compiler to inline functions, I need compiler flag?

// The IR sensors is high by default (when inactive)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    if (P1IFG & BIT5) {
        // TODO: According to the DATASHEET, the timer should be stopped while reading timer register!)
        time_passed = (time_total + TA0R) - time_last_interrupt;
        time_last_interrupt = (time_total + TA0R);
        P1IE &= ~BIT5; // Disable interrupt while processing
        if ((bitcounter == 32) && (ir_state == STATE_RECEIVING_BITS || ir_state == STATE_REPEATING)) {
            // We only get to here if the user keeps a key pressed
            if (time_last_interrupt > (T_REPEATING_USEC-T_ERROR_MARGIN_USEC)) { // TODO: set upper limit too?
                repeating += 1;
                if (repeating > 19) {  // TODO: Should it be repeating one here? shouldn't that be the next pulse?
                    repeating = repeating;
                }
                ir_remote_msg_buffer_add(message);
                time_total = 0;
                time_last_interrupt = 0;
                time_passed = 0;
                ir_state = STATE_REPEATING; // TODO: Don't really need this state (can use inactive instead), but maybe good for readability?
            } else {
                if (ir_state == STATE_RECEIVING_BITS) {
                    ir_remote_reset();
                }
                else {
                    // Might be the start of a normal message
                    // TODO: Check if this is actually handled correctly, might need to
                    //       reset message, timer etc...
                }
            }
        }

        switch (ir_state) {
        case STATE_INACTIVE:
        case STATE_REPEATING:
            TACTL |= TACLR;
            TACTL = TASSEL_2 + MC_1; // SMCLK, up mode
            time_total = 0;
            P1IES &= ~BIT5; // rising edge
            ir_state = STATE_AGC;
            break;
        case STATE_AGC:
            if (time_passed > (T_AGC_USEC - T_ERROR_MARGIN_USEC))
            {
                // AGC received
                // Next we expect a fall in ~4.5 ms
                P1IES |= BIT5;
                ir_state = STATE_AFTER_AGC;
            } else {
                ir_remote_reset();
            }
            break;
        case STATE_AFTER_AGC:
            // Fall after ~4.5ms
            if (time_passed > (T_AFTER_AGC_USEC - T_ERROR_MARGIN_USEC)) {
                ir_state = STATE_RECEIVING_BITS;
                // WE expect a rising edge after ~0.6ms
                // P1IES &= ~BIT5; // rising edge
                // But we want to look for the falling edge and measure that time to determine 0 or 1
                P1IES |= BIT5;
            } else if (repeating && (time_passed > (T_REPEATING_AFTER_AGC_USEC - T_ERROR_MARGIN_USEC))) {
                ir_state = STATE_REPEATING;
                P1IES |= BIT5;
            } else {
                ir_remote_reset();
            }
            break;
        case STATE_RECEIVING_BITS:
            if ((time_passed > (T_LOGICAL_ZERO_USEC - T_ERROR_MARGIN_USEC)) && (time_passed < (T_LOGICAL_ZERO_USEC + T_ERROR_MARGIN_USEC))) {
                // A zero
                bitcounter++;
                message <<= 1;

            } else if ((time_passed > (T_LOGICAL_ONE_USEC - T_ERROR_MARGIN_USEC)) && (time_passed < (T_LOGICAL_ONE_USEC + T_ERROR_MARGIN_USEC))) {
                // A one
                bitcounter++;
                message <<= 1;
                message |= 0x1;
            } else {
                ir_remote_reset();
            }
            if (bitcounter == 16) {
                bitcounter = 16;
            }
            if (bitcounter == 32) {
                bitcounter = 32;
                ir_remote_msg_buffer_add(message);
            }
            break;
        }

        P1IFG &= ~BIT5; // Clear
        P1IE |= BIT5; // Enable again
    }
}

// TODO: Change BIT5 to GPIO name
void ir_remote_init()
{
    P1REN |= BIT5;
    P1OUT |= BIT5;
    P1IE |= BIT5; // P1.1 interrupt enabled
    P1IES |= BIT5; // P1.1 high/low edge
    P1IFG &= ~BIT5; // P1.1 IFG cleared

    TACCTL0 = CCIE;  // TACCR0 interrupt enabled
    TACTL = TASSEL_2 + MC_1; // SMCLK, up mode
    TACCR0 = 65000; // interrupt after 65ms
}

ir_remote_command_t ir_remote_get_command()
{
    // For now disable interrupts while getting messages
    // This can possibly cause missed flanks and in turn IR messages
    // TODO: Investigate alternative solution to disabling interrupts
    P1IE &= ~BIT5;
    ir_remote_command_t command = COMMAND_NONE;
    if (msg_buffer_head != msg_buffer_tail) {
        // buffer not empty
        command = msg_to_cmd(msg_buffer[msg_buffer_tail]);
        msg_buffer_tail += 1;
        if (msg_buffer_tail >= MSG_BUFFER_SIZE) {
            msg_buffer_tail = 0;
        }

    }

    P1IE |= BIT5;
    return command;
}
