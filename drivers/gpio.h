#ifndef GPIO_H
#define GPIO_H

#include <msp430.h>

/* This file contains helper definitions and helper functions to make the pin handling easier. */

#define GPIO_PORT_OFFSET      (8u)
#define GPIO_PORT_MASK        (0xFFu << GPIO_PORT_OFFSET)
#define GPIO_PORT(gpio)       ((gpio & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
#define GPIO_PIN_MASK         (0xFFu)
#define GPIO_PIN(gpio)        (gpio & GPIO_PIN_MASK)
#define GPIO_MAKE(port, pin)  ((port << GPIO_PORT_OFFSET) | pin)
#define GPIO_PORT_CNT         (2u)
#define GPIO_PIN_CNT_PER_PORT (8u)

#define GPIO_10 GPIO_MAKE(GPIO_PORT_1, BIT0)
#define GPIO_11 GPIO_MAKE(GPIO_PORT_1, BIT1)
#define GPIO_12 GPIO_MAKE(GPIO_PORT_1, BIT2)
#define GPIO_13 GPIO_MAKE(GPIO_PORT_1, BIT3)
#define GPIO_14 GPIO_MAKE(GPIO_PORT_1, BIT4)
#define GPIO_15 GPIO_MAKE(GPIO_PORT_1, BIT5)
#define GPIO_16 GPIO_MAKE(GPIO_PORT_1, BIT6)
#define GPIO_17 GPIO_MAKE(GPIO_PORT_1, BIT7)

#define GPIO_20 GPIO_MAKE(GPIO_PORT_2, BIT0)
#define GPIO_21 GPIO_MAKE(GPIO_PORT_2, BIT1)
#define GPIO_22 GPIO_MAKE(GPIO_PORT_2, BIT2)
#define GPIO_23 GPIO_MAKE(GPIO_PORT_2, BIT3)
#define GPIO_24 GPIO_MAKE(GPIO_PORT_2, BIT4)
#define GPIO_25 GPIO_MAKE(GPIO_PORT_2, BIT5)
#define GPIO_26 GPIO_MAKE(GPIO_PORT_2, BIT6)
#define GPIO_27 GPIO_MAKE(GPIO_PORT_2, BIT7)

typedef enum {
    GPIO_PORT_1,
    GPIO_PORT_2
} gpio_port_t;

typedef enum {
    RESISTOR_DISABLED,
    RESISTOR_ENABLED
} gpio_resistor_t;

typedef enum {
    GPIO_OUTPUT,
    GPIO_INPUT
} gpio_dir_t;

typedef enum {
    GPIO_LOW,
    GPIO_HIGH
} gpio_output_t;

typedef enum {
    GPIO_SEL_GPIO,
    GPIO_SEL_1,
    GPIO_SEL_2,
    GPIO_SEL_3
} gpio_selection_t;

typedef enum {
    /* Port 1 */
    GPIO_ADC_LEFT_SENSOR = GPIO_10,
    GPIO_UART_RXD = GPIO_11,
    GPIO_UART_TXD = GPIO_12,
    GPIO_ADC_FRONT_LEFT_SENSOR = GPIO_13,
    GPIO_ADC_FRONT_SENSOR = GPIO_14,
    GPIO_ADC_FRONT_RIGHT_SENSOR = GPIO_15,
    GPIO_ADC_RIGHT_SENSOR = GPIO_16,
    GPIO_P17 = GPIO_17,
    /* Port 2 */
    GPIO_MOTORS_LEFT_CC_1 = GPIO_20,
    GPIO_PWM_0 = GPIO_21,
    GPIO_MOTORS_LEFT_CC_2 = GPIO_22,
    GPIO_MOTORS_RIGHT_CC_1 = GPIO_23,
    GPIO_PWM_1 = GPIO_24,
    GPIO_MOTORS_RIGHT_CC_2 = GPIO_25,
    GPIO_P26 = GPIO_26,
    GPIO_IR_REMOTE = GPIO_27,
} gpio_t;

typedef struct {
    gpio_t gpio;
    gpio_dir_t dir;
    gpio_output_t out;
    gpio_resistor_t resistor;
    gpio_selection_t selection;
} gpio_config_t;

typedef enum {
    TRIGGER_RISING,
    TRIGGER_FALLING,
} trigger_t;

void gpio_init();
void gpio_configure(const gpio_config_t* config);
void gpio_set_direction(gpio_t gpio, gpio_dir_t direction);
void gpio_set_output(gpio_t gpio, gpio_output_t output);
void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor);
void gpio_set_selection(gpio_t gpio, gpio_selection_t selection);

void gpio_enable_interrupt(gpio_t gpio);
void gpio_disable_interrupt(gpio_t gpio);
void gpio_set_interrupt_trigger(gpio_t gpio, trigger_t trigger);
typedef void (*isr_function)(void);
void gpio_register_isr(gpio_t gpio, isr_function isr);

#endif /* GPIO_H */
