#include "ir_remote.h"
#include "gpio.h"
#include <msp430.h>
#include <stdint.h>

/* NEC protocol */
#define T_AGC_USEC 9000
#define T_AFTER_AGC_USEC 4500
#define T_LOGICAL_ONE_USEC 2250
#define T_LOGICAL_ZERO_USEC 1125
#define T_ERROR_MARGIN_USEC 250
#define T_MAX_USEC 65000
/* 110 ms according to spec, but get ~108600 uS when measuring */
#define T_REPEATING_USEC 108600
#define T_REPEATING_AFTER_AGC_USEC 2250

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
static volatile uint16_t msg_buffer_tail = 0;
static volatile uint16_t bit_counter = 0;
static volatile uint32_t message = 0;
static volatile uint32_t time_total = 0;
static volatile uint32_t time_passed = 0;
static volatile uint32_t time_last_interrupt = 0;
static volatile uint16_t repeating = 0;
static volatile ir_state_t ir_state = STATE_INACTIVE;

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

static inline void ir_remote_msg_buffer_add(uint32_t msg)
{
    msg_buffer[msg_buffer_head++] = msg;
    if (msg_buffer_head >= MSG_BUFFER_SIZE) {
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
    TACTL = MC_0 + TACLR;
    ir_state = STATE_INACTIVE;
    bit_counter = 0;
    time_total = 0;
    time_last_interrupt = 0;
    time_passed = 0;
    repeating = 0;
    message = 0;
    gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
}

void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void) {
    if (time_total == T_MAX_USEC) {
        /* Timeout */
        ir_remote_reset();
    } else {
        time_total += T_MAX_USEC;
    }
}

/* This ISR is kind of large and we could just collect the flanks here and
 * do the processing inside the main loop. But it's not that bad (~20-120 uS)
 * and the remote controlling is just for testing, so leave it as is for now. */
static void ir_remote_isr() {
    /* TODO: We should stop the timer while reading TA0R (see datasheet) */
    time_passed = (time_total + TA0R) - time_last_interrupt;
    time_last_interrupt = (time_total + TA0R);
    if ((bit_counter == 32) && (ir_state == STATE_RECEIVING_BITS || ir_state == STATE_REPEATING)) {
        /* We only get to here if the user keeps a key pressed */
        /* TODO: set upper limit too? */
        if (time_last_interrupt > (T_REPEATING_USEC - T_ERROR_MARGIN_USEC)) {
            repeating += 1;
            ir_remote_msg_buffer_add(message);
            time_total = 0;
            time_last_interrupt = 0;
            time_passed = 0;
            ir_state = STATE_REPEATING;
        } else {
            if (ir_state == STATE_RECEIVING_BITS) {
                ir_remote_reset();
            } else {
                /* TODO: How to handle? */
            }
        }
    }

    switch (ir_state) {
    case STATE_INACTIVE:
    case STATE_REPEATING:
        TACTL |= TACLR;
        TACTL = TASSEL_2 + MC_1;
        time_total = 0;
        gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_RISING);
        ir_state = STATE_AGC;
        break;
    case STATE_AGC:
        if (time_passed > (T_AGC_USEC - T_ERROR_MARGIN_USEC)) {
            /* We received AGC, so next we next we expect a fall in ~4.5 ms */
            gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
            ir_state = STATE_AFTER_AGC;
        } else {
            /* Unexpected, reset */
            ir_remote_reset();
        }
        break;
    case STATE_AFTER_AGC:
        if (time_passed > (T_AFTER_AGC_USEC - T_ERROR_MARGIN_USEC)) {
            ir_state = STATE_RECEIVING_BITS;
            /* Look for the falling edge and then measure the time to determine 0 or 1 */
            gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
        } else if (repeating && (time_passed > (T_REPEATING_AFTER_AGC_USEC - T_ERROR_MARGIN_USEC))) {
            ir_state = STATE_REPEATING;
            gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
        } else {
            /* Unexpected, reset */
            ir_remote_reset();
        }
        break;
    case STATE_RECEIVING_BITS:
        if ((time_passed > (T_LOGICAL_ZERO_USEC - T_ERROR_MARGIN_USEC)) && (time_passed < (T_LOGICAL_ZERO_USEC + T_ERROR_MARGIN_USEC))) {
            /* We got a 0 */
            bit_counter++;
            message <<= 1;
        } else if ((time_passed > (T_LOGICAL_ONE_USEC - T_ERROR_MARGIN_USEC)) && (time_passed < (T_LOGICAL_ONE_USEC + T_ERROR_MARGIN_USEC))) {
            /* We got a 1 */
            bit_counter++;
            message <<= 1;
            message |= 0x1;
        } else {
            /* Unexpected, rest */
            ir_remote_reset();
        }
        if (bit_counter == 32) {
            /* We got all bits */
            ir_remote_msg_buffer_add(message);
        }
        break;
    }
}

void ir_remote_init()
{
    gpio_enable_interrupt(GPIO_IR_REMOTE);
    gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
    gpio_register_isr(GPIO_IR_REMOTE, ir_remote_isr);

    TACCTL0 = CCIE;  /* TACCR0 interrupt enabled */
    TACTL = TASSEL_2 + MC_1; /* SMCLK, up mode */
    TACCR0 = T_MAX_USEC; /* Interrupt after 65 us */
}

ir_remote_command_t ir_remote_get_command()
{
    gpio_disable_interrupt(GPIO_IR_REMOTE);
    ir_remote_command_t command = COMMAND_NONE;
    if (msg_buffer_head != msg_buffer_tail) {
        command = msg_to_cmd(msg_buffer[msg_buffer_tail]);
        msg_buffer_tail += 1;
        if (msg_buffer_tail >= MSG_BUFFER_SIZE) {
            msg_buffer_tail = 0;
        }
    }
    gpio_enable_interrupt(GPIO_IR_REMOTE);
    return command;
}
