#ifndef GPIO_H
#define GPIO_H

#include <msp430.h>

/* This file contains helper definitions and helper functions to make GPIO handling easier. */

#define GPIO_PORT_OFFSET     (8u)
#define GPIO_PORT_MASK       (0xFFu << GPIO_PORT_OFFSET)
#define GPIO_PORT(gpio)      ((gpio & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
#define GPIO_PIN_MASK        (0xFFu)
#define GPIO_PIN(gpio)       (gpio & GPIO_PIN_MASK)
#define GPIO_MAKE(port, pin) ((port << GPIO_PORT_OFFSET) | pin)

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
    GPIO_ADC_LEFT_SENSOR = GPIO_MAKE(GPIO_PORT_1, BIT0), /* 1.0 */
    GPIO_UART_RXD = GPIO_MAKE(GPIO_PORT_1, BIT1), /* 1.1 */
    GPIO_UART_TXD = GPIO_MAKE(GPIO_PORT_1, BIT2), /* 1.2 (also used for USB transfer) */
    GPIO_ADC_FRONT_LEFT_SENSOR = GPIO_MAKE(GPIO_PORT_1, BIT3), /* 1.3 */
    GPIO_ADC_FRONT_SENSOR = GPIO_MAKE(GPIO_PORT_1, BIT4), /* 1.4 */
    GPIO_ADC_FRONT_RIGHT_SENSOR = GPIO_MAKE(GPIO_PORT_1, BIT5), /* 1.5 */
    GPIO_ADC_RIGHT_SENSOR = GPIO_MAKE(GPIO_PORT_1, BIT6), /* 1.6 */
    GPIO_P17 = GPIO_MAKE(GPIO_PORT_1, BIT7), /* 1.7 */
    /* Port 2 */
    GPIO_MOTORS_LEFT_CC_1 = GPIO_MAKE(GPIO_PORT_2, BIT0), /* 1.0 */
    GPIO_PWM_0 = GPIO_MAKE(GPIO_PORT_2, BIT1),  /* 1.1 */
    GPIO_MOTORS_LEFT_CC_2 = GPIO_MAKE(GPIO_PORT_2, BIT2), /* 1.2 */
    GPIO_MOTORS_RIGHT_CC_1 = GPIO_MAKE(GPIO_PORT_2, BIT3), /* 1.3 */
    GPIO_PWM_1 = GPIO_MAKE(GPIO_PORT_2, BIT4), /* 1.4 */
    GPIO_MOTORS_RIGHT_CC_2 = GPIO_MAKE(GPIO_PORT_2, BIT5), /* 1.5 */
    GPIO_P26 = GPIO_MAKE(GPIO_PORT_2, BIT6), /* 1.6 */
    GPIO_P27 = GPIO_MAKE(GPIO_PORT_2, BIT7) /* 1.7 */
} gpio_t;

typedef struct {
    gpio_t gpio;
    gpio_dir_t dir;
    gpio_output_t out;
    gpio_resistor_t resistor;
    gpio_selection_t selection;
} gpio_config_t;

void gpio_init();
void gpio_configure(const gpio_config_t* config);
void gpio_set_direction(gpio_t gpio, gpio_dir_t direction);
void gpio_set_output(gpio_t gpio, gpio_output_t output);
void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor);
void gpio_set_selection(gpio_t gpio, gpio_selection_t selection);

#endif /* GPIO_H */
