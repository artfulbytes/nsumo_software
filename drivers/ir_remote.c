#include "ir_remote.h"
#include "gpio.h"
#include <msp430.h>
#include <stdint.h>

/**
 * NEC protocol
 * https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
 */
#define T_AGC_USEC (9000)
#define T_AFTER_AGC_USEC (4500)
#define T_LOGICAL_ONE_USEC (2250)
#define T_LOGICAL_ZERO_USEC (1125)
#define T_ERROR_MARGIN_USEC (250)
#define T_REPEATING_USEC (108000)
#define T_REPEATING_AFTER_AGC_USEC (2250)

/* Timer defines */
#define T_MAX_TICKS (65000)
/* SMCLK = 16 MHz, TA1 input divider = 8 */
#define TICK_TO_USEC_DIVISOR (16 / 8)
#define T_MAX_USEC (T_MAX_TICKS / TICK_TO_USEC_DIVISOR)
#define T_FAIL_TIMEOUT_USEC (T_REPEATING_USEC + T_ERROR_MARGIN_USEC)
#define TA1CTL_CONFIG_FLAGS (TASSEL_2 + MC_1 + ID_3) /* SMCLK / 8, up mode */

#define MSG_BUFFER_SIZE (10)

typedef enum
{
    STATE_INACTIVE,
    STATE_AGC,
    STATE_AFTER_AGC,
    STATE_RECEIVING_BITS,
    STATE_REPEATING
} ir_state_t;

static volatile uint32_t msg_buffer[MSG_BUFFER_SIZE] = { 0 };
static volatile uint16_t msg_buffer_head = 0;
static volatile uint16_t msg_buffer_tail = 0;
static volatile uint16_t bit_counter = 0;
static volatile uint32_t message = 0;
static volatile uint32_t timer_base_usec = 0;
static volatile uint32_t time_passed_usec = 0;
static volatile uint32_t time_last_interrupt_usec = 0;
static volatile uint16_t repeating = 0;
static volatile ir_state_t ir_state = STATE_INACTIVE;

static ir_key_t msg_to_key(uint32_t msg)
{
    // TODO: Can save ~150 bytes by just checking 16 LSB bits
    switch (msg) {
    case 16750695:
        return IR_KEY_0;
    case 16753245:
        return IR_KEY_1;
    case 16736925:
        return IR_KEY_2;
    case 16769565:
        return IR_KEY_3;
    case 16720605:
        return IR_KEY_4;
    case 16712445:
        return IR_KEY_5;
    case 16761405:
        return IR_KEY_6;
    case 16769055:
        return IR_KEY_7;
    case 16754775:
        return IR_KEY_8;
    case 16748655:
        return IR_KEY_9;
    case 16738455:
        return IR_KEY_STAR;
    case 16756815:
        return IR_KEY_HASH;
    case 16718055:
        return IR_KEY_UP;
    case 16730805:
        return IR_KEY_DOWN;
    case 16716015:
        return IR_KEY_LEFT;
    case 16734885:
        return IR_KEY_RIGHT;
    case 16726215:
        return IR_KEY_OK;
    }
    return IR_KEY_NONE;
}

static void ir_remote_msg_buffer_add(uint32_t msg)
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

static void ir_remote_reset()
{
    TA1CTL = MC_0 + TACLR;
    ir_state = STATE_INACTIVE;
    bit_counter = 0;
    timer_base_usec = 0;
    time_last_interrupt_usec = 0;
    time_passed_usec = 0;
    repeating = 0;
    message = 0;
    gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
}

static void restart_timer()
{
    TA1CTL |= TACLR;
    TA1CTL = TA1CTL_CONFIG_FLAGS;
    timer_base_usec = 0;
    time_passed_usec = 0;
    time_last_interrupt_usec = 0;
}

void __attribute__((interrupt(TIMER1_A0_VECTOR))) Timer_A(void)
{
    timer_base_usec += T_MAX_USEC;
    if (timer_base_usec >= T_FAIL_TIMEOUT_USEC) {
        ir_remote_reset();
    }
}

static bool time_equal(uint32_t t1, uint32_t t2, uint32_t error_margin)
{
    return (t1 > (t2 - error_margin)) && (t1 < (t2 + error_margin));
}

/* This ISR is kind of large and to make it smaller we could just collect the flanks
 * here and do the processing inside the main loop. But it's not that bad (~20-120 uS)
 * and the remote controlling is mainly for testing, so leave it as is for now. */
static void ir_remote_isr()
{
    /* TODO: Reading TA1R while timer is running may be dangerous (see datasheet) */
    const uint16_t timer_usec = TA1R / TICK_TO_USEC_DIVISOR;
    const uint32_t timer_total_usec = timer_base_usec + timer_usec;
    time_passed_usec = timer_total_usec - time_last_interrupt_usec;
    time_last_interrupt_usec = timer_total_usec;

    if ((bit_counter == 32) && (ir_state == STATE_RECEIVING_BITS || ir_state == STATE_REPEATING)) {
        /* We only get to here if the user keeps a key pressed */
        if (time_equal(timer_total_usec, T_REPEATING_USEC, T_ERROR_MARGIN_USEC)) {
            repeating += 1;
            ir_remote_msg_buffer_add(message);
            ir_state = STATE_REPEATING;
        } else {
            if (ir_state == STATE_RECEIVING_BITS) {
                ir_remote_reset();
            } else {
                /* TODO: How to handle? assert? */
            }
        }
    }

    switch (ir_state) {
    case STATE_INACTIVE:
    case STATE_REPEATING:
        restart_timer();
        gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_RISING);
        ir_state = STATE_AGC;
        break;
    case STATE_AGC:
        if (time_equal(time_passed_usec, T_AGC_USEC, T_ERROR_MARGIN_USEC)) {
            /* We received AGC, so next we next we expect a fall in ~4.5 ms */
            gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
            ir_state = STATE_AFTER_AGC;
        } else {
            /* Unexpected, reset */
            ir_remote_reset();
        }
        break;
    case STATE_AFTER_AGC:
        if (time_equal(time_passed_usec, T_AFTER_AGC_USEC, T_ERROR_MARGIN_USEC)) {
            ir_state = STATE_RECEIVING_BITS;
            /* Look for the falling edge and then measure the time to determine 0 or 1 */
            gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
        } else if (repeating
                   && (time_passed_usec > (T_REPEATING_AFTER_AGC_USEC - T_ERROR_MARGIN_USEC))) {
            ir_state = STATE_REPEATING;
            gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
        } else {
            /* Unexpected, reset */
            ir_remote_reset();
        }
        break;
    case STATE_RECEIVING_BITS:
        if (time_equal(time_passed_usec, T_LOGICAL_ZERO_USEC, T_ERROR_MARGIN_USEC)) {
            /* We got a 0 */
            bit_counter++;
            message <<= 1;
        } else if (time_equal(time_passed_usec, T_LOGICAL_ONE_USEC, T_ERROR_MARGIN_USEC)) {
            /* We got a 1 */
            bit_counter++;
            message <<= 1;
            message |= 0x1;
        } else {
            /* Unexpected, reset */
            ir_remote_reset();
        }
        if (bit_counter == 32) {
            /* We got all bits */
            ir_remote_msg_buffer_add(message);
        }
        break;
    }
}

ir_key_t ir_remote_get_key()
{
    gpio_disable_interrupt(GPIO_IR_REMOTE);
    ir_key_t key = IR_KEY_NONE;
    if (msg_buffer_head != msg_buffer_tail) {
        key = msg_to_key(msg_buffer[msg_buffer_tail]);
        msg_buffer_tail += 1;
        if (msg_buffer_tail >= MSG_BUFFER_SIZE) {
            msg_buffer_tail = 0;
        }
    }
    gpio_enable_interrupt(GPIO_IR_REMOTE);
    return key;
}

void ir_remote_wait_for_start_signal()
{
    /* For now, let any keypress from the remote control represent a
     * start signal */
    ir_remote_init();
    while (ir_remote_get_key() == IR_KEY_NONE)
        ;
}

static bool inited = false;
void ir_remote_init()
{
    if (inited) {
        return;
    }
    gpio_enable_interrupt(GPIO_IR_REMOTE);
    gpio_set_interrupt_trigger(GPIO_IR_REMOTE, TRIGGER_FALLING);
    gpio_register_isr(GPIO_IR_REMOTE, ir_remote_isr);

    TA1CCTL0 = CCIE; /* TACCR0 interrupt enabled */
    TA1CTL = TA1CTL_CONFIG_FLAGS;
    TA1CCR0 = T_MAX_TICKS; /* (Interrupt after T_MAX_USEC) */
    inited = true;
}
