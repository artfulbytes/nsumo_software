#include "gpio.h"
#include "defines.h"
#include <msp430.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static volatile uint8_t *port_dir_registers[] = { &P1DIR, &P2DIR, &P3DIR };
static volatile uint8_t *port_ren_registers[] = { &P1REN, &P2REN, &P3REN };

static volatile uint8_t *port_out_registers[] = { &P1OUT, &P2OUT, &P3OUT };

static volatile uint8_t *port_in_registers[] = { &P1IN, &P2IN, &P3IN };

static volatile uint8_t *port_sel_registers[] = {
    &P1SEL, &P1SEL2, &P2SEL, &P2SEL2, &P3SEL, &P3SEL2
};

static volatile uint8_t *port_interrupt_flag_registers[] = {
    &P1IFG, &P2IFG
    /* Port 3 is not interrupt capable on MSP430G2553 */
};

static volatile uint8_t *port_interrupt_enable_registers[] = {
    &P1IE, &P2IE
    /* Port 3 is not interrupt capable on MSP430G2553 */
};

static volatile uint8_t *port_edge_select_registers[] = {
    &P1IES, &P2IES
    /* Port 3 is not interrupt capable on MSP430G2553 */
};

static const gpio_config_t gpio_initial_config[] = {
    { GPIO_IR_REMOTE, GPIO_INPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_UART_RXD, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_3 },
    { GPIO_UART_TXD, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_3 },
    { GPIO_UNUSED_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_1 },
    { GPIO_UNUSED_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_1 },
    { GPIO_XSHUT_RIGHT, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_I2C_SCL, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_3 },
    { GPIO_I2C_SDA, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_3 },

    { GPIO_RANGE_SENSOR_FRONT_INT, GPIO_INPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_XSHUT_FRONT, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_XSHUT_FRONT_LEFT, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_LINE_DETECT_BACK_LEFT, GPIO_INPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_MOTORS_LEFT_CC_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_MOTORS_LEFT_CC_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_TEST_LED, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_MOTORS_RIGHT_CC_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },

    { GPIO_XSHUT_FRONT_RIGHT, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_LINE_DETECT_FRONT_RIGHT, GPIO_INPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_LINE_DETECT_FRONT_LEFT, GPIO_INPUT, GPIO_HIGH, RESISTOR_ENABLED, GPIO_SEL_GPIO },
    { GPIO_XSHUT_LEFT, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_LINE_DETECT_BACK_RIGHT, GPIO_INPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_PWM_MOTORS_LEFT, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_PWM_MOTORS_RIGHT, GPIO_OUTPUT, GPIO_HIGH, RESISTOR_DISABLED, GPIO_SEL_GPIO },
    { GPIO_MOTORS_RIGHT_CC_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO },
};

static volatile isr_function isr_functions[GPIO_PORT_CNT][GPIO_PIN_CNT_PER_PORT] = {
    [GPIO_PORT_1] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    [GPIO_PORT_2] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    [GPIO_PORT_3] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

static inline void gpio_clear_interrupt(gpio_t gpio)
{
    *port_interrupt_flag_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
}

void gpio_init()
{
    *port_interrupt_enable_registers[GPIO_PORT_1] &= 0;
    *port_interrupt_enable_registers[GPIO_PORT_2] &= 0;

    for (uint16_t cfg_idx = 0; cfg_idx < ARRAY_SIZE(gpio_initial_config); cfg_idx++) {
        gpio_configure(&gpio_initial_config[cfg_idx]);
    }
}

void gpio_configure(const gpio_config_t *config)
{
    gpio_set_direction(config->gpio, config->dir);
    gpio_set_output(config->gpio, config->out);
    gpio_set_resistor(config->gpio, config->resistor);
    gpio_set_selection(config->gpio, config->selection);
}

void gpio_set_direction(gpio_t gpio, gpio_dir_t direction)
{
    switch (direction) {
    case GPIO_OUTPUT:
        *port_dir_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    case GPIO_INPUT:
        *port_dir_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
        break;
    }
}

void gpio_set_output(gpio_t gpio, gpio_output_t output)
{
    switch (output) {
    case GPIO_HIGH:
        *port_out_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    case GPIO_LOW:
        *port_out_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
        break;
    }
}

bool gpio_get_input(gpio_t gpio) { return *port_in_registers[GPIO_PORT(gpio)] &= GPIO_PIN(gpio); }

void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor)
{
    switch (resistor) {
    case RESISTOR_ENABLED:
        *port_ren_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    case RESISTOR_DISABLED:
        *port_ren_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
        break;
    }
}

void gpio_set_selection(gpio_t gpio, gpio_selection_t selection)
{
    switch (selection) {
    case GPIO_SEL_GPIO:
        *port_sel_registers[GPIO_PORT(gpio) * 2] &= ~GPIO_PIN(gpio);
        *port_sel_registers[GPIO_PORT(gpio) * 2 + 1] &= ~GPIO_PIN(gpio);
        break;
    case GPIO_SEL_1:
        *port_sel_registers[GPIO_PORT(gpio) * 2] |= GPIO_PIN(gpio);
        *port_sel_registers[GPIO_PORT(gpio) * 2 + 1] &= ~GPIO_PIN(gpio);
        break;
    case GPIO_SEL_2:
        *port_sel_registers[GPIO_PORT(gpio) * 2] &= ~GPIO_PIN(gpio);
        *port_sel_registers[GPIO_PORT(gpio) * 2 + 1] |= GPIO_PIN(gpio);
        break;
    case GPIO_SEL_3:
        *port_sel_registers[GPIO_PORT(gpio) * 2] |= GPIO_PIN(gpio);
        *port_sel_registers[GPIO_PORT(gpio) * 2 + 1] |= GPIO_PIN(gpio);
        break;
    }
}

/* On the MSP430 the P1IFG bits are set when a flank occurs even if the
 * corresponding bit in P1IE is disabled. Disabling P1IE only prevents it
 * from triggering an interrupt. */
void __attribute__((interrupt(PORT1_VECTOR))) Port_1(void)
{
    if (P1IFG & GPIO_PIN(GPIO_10)) {
        if (isr_functions[GPIO_PORT(GPIO_10)][GPIO_PIN_IDX(GPIO_10)]) {
            isr_functions[GPIO_PORT(GPIO_10)][GPIO_PIN_IDX(GPIO_10)]();
        }
        gpio_clear_interrupt(GPIO_10);
    }
    if (P1IFG & GPIO_PIN(GPIO_11)) {
        if (isr_functions[GPIO_PORT(GPIO_11)][GPIO_PIN_IDX(GPIO_11)]) {
            isr_functions[GPIO_PORT(GPIO_11)][GPIO_PIN_IDX(GPIO_11)]();
        }
        gpio_clear_interrupt(GPIO_11);
    }
    if (P1IFG & GPIO_PIN(GPIO_12)) {
        if (isr_functions[GPIO_PORT(GPIO_12)][GPIO_PIN_IDX(GPIO_12)]) {
            isr_functions[GPIO_PORT(GPIO_12)][GPIO_PIN_IDX(GPIO_12)]();
        }
        gpio_clear_interrupt(GPIO_12);
    }
    if (P1IFG & GPIO_PIN(GPIO_13)) {
        if (isr_functions[GPIO_PORT(GPIO_13)][GPIO_PIN_IDX(GPIO_13)]) {
            isr_functions[GPIO_PORT(GPIO_13)][GPIO_PIN_IDX(GPIO_13)]();
        }
        gpio_clear_interrupt(GPIO_13);
    }
    if (P1IFG & GPIO_PIN(GPIO_14)) {
        if (isr_functions[GPIO_PORT(GPIO_14)][GPIO_PIN_IDX(GPIO_14)]) {
            isr_functions[GPIO_PORT(GPIO_14)][GPIO_PIN_IDX(GPIO_14)]();
        }
        gpio_clear_interrupt(GPIO_14);
    }
    if (P1IFG & GPIO_PIN(GPIO_15)) {
        if (isr_functions[GPIO_PORT(GPIO_15)][GPIO_PIN_IDX(GPIO_15)]) {
            isr_functions[GPIO_PORT(GPIO_15)][GPIO_PIN_IDX(GPIO_15)]();
        }
        gpio_clear_interrupt(GPIO_15);
    }
    if (P1IFG & GPIO_PIN(GPIO_16)) {
        if (isr_functions[GPIO_PORT(GPIO_16)][GPIO_PIN_IDX(GPIO_16)]) {
            isr_functions[GPIO_PORT(GPIO_16)][GPIO_PIN_IDX(GPIO_16)]();
        }
        gpio_clear_interrupt(GPIO_16);
    }
    if (P1IFG & GPIO_PIN(GPIO_17)) {
        if (isr_functions[GPIO_PORT(GPIO_17)][GPIO_PIN_IDX(GPIO_17)]) {
            isr_functions[GPIO_PORT(GPIO_17)][GPIO_PIN_IDX(GPIO_17)]();
        }
        gpio_clear_interrupt(GPIO_17);
    }
}

/* On the MSP430 the P2IFG bits are set when a flank occurs even if the
 * corresponding bit in P2IE is disabled. Disabling the bit in P2IE only
 * prevents it from triggering an interrupt. */
void __attribute__((interrupt(PORT2_VECTOR))) Port_2(void)
{
    if (P2IFG & GPIO_PIN(GPIO_20)) {
        if (isr_functions[GPIO_PORT(GPIO_20)][GPIO_PIN_IDX(GPIO_20)]) {
            isr_functions[GPIO_PORT(GPIO_20)][GPIO_PIN_IDX(GPIO_20)]();
        }
        gpio_clear_interrupt(GPIO_20);
    }
    if (P2IFG & GPIO_PIN(GPIO_21)) {
        if (isr_functions[GPIO_PORT(GPIO_21)][GPIO_PIN_IDX(GPIO_21)]) {
            isr_functions[GPIO_PORT(GPIO_21)][GPIO_PIN_IDX(GPIO_21)]();
        }
        gpio_clear_interrupt(GPIO_21);
    }
    if (P2IFG & GPIO_PIN(GPIO_22)) {
        if (isr_functions[GPIO_PORT(GPIO_22)][GPIO_PIN_IDX(GPIO_22)]) {
            isr_functions[GPIO_PORT(GPIO_22)][GPIO_PIN_IDX(GPIO_22)]();
        }
        gpio_clear_interrupt(GPIO_22);
    }
    if (P2IFG & GPIO_PIN(GPIO_23)) {
        if (isr_functions[GPIO_PORT(GPIO_23)][GPIO_PIN_IDX(GPIO_23)]) {
            isr_functions[GPIO_PORT(GPIO_23)][GPIO_PIN_IDX(GPIO_23)]();
        }
        gpio_clear_interrupt(GPIO_23);
    }
    if (P2IFG & GPIO_PIN(GPIO_24)) {
        if (isr_functions[GPIO_PORT(GPIO_24)][GPIO_PIN_IDX(GPIO_24)]) {
            isr_functions[GPIO_PORT(GPIO_24)][GPIO_PIN_IDX(GPIO_24)]();
        }
        gpio_clear_interrupt(GPIO_24);
    }
    if (P2IFG & GPIO_PIN(GPIO_25)) {
        if (isr_functions[GPIO_PORT(GPIO_25)][GPIO_PIN_IDX(GPIO_25)]) {
            isr_functions[GPIO_PORT(GPIO_25)][GPIO_PIN_IDX(GPIO_25)]();
        }
        gpio_clear_interrupt(GPIO_25);
    }
    if (P2IFG & GPIO_PIN(GPIO_26)) {
        if (isr_functions[GPIO_PORT(GPIO_26)][GPIO_PIN_IDX(GPIO_26)]) {
            isr_functions[GPIO_PORT(GPIO_26)][GPIO_PIN_IDX(GPIO_26)]();
        }
        gpio_clear_interrupt(GPIO_26);
    }
    if (P2IFG & GPIO_PIN(GPIO_27)) {
        if (isr_functions[GPIO_PORT(GPIO_27)][GPIO_PIN_IDX(GPIO_27)]) {
            isr_functions[GPIO_PORT(GPIO_27)][GPIO_PIN_IDX(GPIO_27)]();
        }
        gpio_clear_interrupt(GPIO_27);
    }
}

void gpio_register_isr(gpio_t gpio, isr_function isr)
{
    isr_functions[GPIO_PORT(gpio)][GPIO_PIN_IDX(gpio)] = isr;
}

void gpio_enable_interrupt(gpio_t gpio)
{
    volatile uint16_t pin_idx = GPIO_PIN(gpio);
    *port_interrupt_enable_registers[GPIO_PORT(gpio)] |= pin_idx;
}

void gpio_disable_interrupt(gpio_t gpio)
{
    *port_interrupt_enable_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
}

void gpio_set_interrupt_trigger(gpio_t gpio, trigger_t trigger)
{
    switch (trigger) {
    case TRIGGER_RISING:
        *port_edge_select_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
        break;
    case TRIGGER_FALLING:
        *port_edge_select_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    }
}
