#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
/* This file contains helper definitions and helper functions to make the pin handling easier. */

/* Each GPIO pin is represented as a uint16_t containing
 * [PIN_IDX (4-bit) | PORT (4-bit) | PIN (8-bit)] */
#define GPIO_PIN_IDX_OFFSET (12u)
#define GPIO_PIN_IDX_MASK (0xFu << GPIO_PIN_IDX_OFFSET)
#define GPIO_PIN_IDX(gpio) ((gpio & GPIO_PIN_IDX_MASK) >> GPIO_PIN_IDX_OFFSET)
#define GPIO_PORT_OFFSET (8u)
#define GPIO_PORT_MASK (0xFu << GPIO_PORT_OFFSET)
#define GPIO_PORT(gpio) ((gpio & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
#define GPIO_PIN_MASK (0xFFu)
#define GPIO_PIN(gpio) (gpio & GPIO_PIN_MASK)
#define GPIO_PIN_IDX_TO_BIT(pin_idx) (1 << pin_idx)
#define GPIO_MAKE(pin_idx, port)                                                                   \
    ((pin_idx << GPIO_PIN_IDX_OFFSET) | (port << GPIO_PORT_OFFSET) | GPIO_PIN_IDX_TO_BIT(pin_idx))
#define GPIO_PORT_CNT (3u)
#define GPIO_PIN_CNT_PER_PORT (8u)

#define GPIO_10 GPIO_MAKE(0, GPIO_PORT_1)
#define GPIO_11 GPIO_MAKE(1, GPIO_PORT_1)
#define GPIO_12 GPIO_MAKE(2, GPIO_PORT_1)
#define GPIO_13 GPIO_MAKE(3, GPIO_PORT_1)
#define GPIO_14 GPIO_MAKE(4, GPIO_PORT_1)
#define GPIO_15 GPIO_MAKE(5, GPIO_PORT_1)
#define GPIO_16 GPIO_MAKE(6, GPIO_PORT_1)
#define GPIO_17 GPIO_MAKE(7, GPIO_PORT_1)

#define GPIO_20 GPIO_MAKE(0, GPIO_PORT_2)
#define GPIO_21 GPIO_MAKE(1, GPIO_PORT_2)
#define GPIO_22 GPIO_MAKE(2, GPIO_PORT_2)
#define GPIO_23 GPIO_MAKE(3, GPIO_PORT_2)
#define GPIO_24 GPIO_MAKE(4, GPIO_PORT_2)
#define GPIO_25 GPIO_MAKE(5, GPIO_PORT_2)
#define GPIO_26 GPIO_MAKE(6, GPIO_PORT_2)
#define GPIO_27 GPIO_MAKE(7, GPIO_PORT_2)

#define GPIO_30 GPIO_MAKE(0, GPIO_PORT_3)
#define GPIO_31 GPIO_MAKE(1, GPIO_PORT_3)
#define GPIO_32 GPIO_MAKE(2, GPIO_PORT_3)
#define GPIO_33 GPIO_MAKE(3, GPIO_PORT_3)
#define GPIO_34 GPIO_MAKE(4, GPIO_PORT_3)
#define GPIO_35 GPIO_MAKE(5, GPIO_PORT_3)
#define GPIO_36 GPIO_MAKE(6, GPIO_PORT_3)
#define GPIO_37 GPIO_MAKE(7, GPIO_PORT_3)

typedef enum
{
    GPIO_PORT_1,
    GPIO_PORT_2,
    GPIO_PORT_3
} gpio_port_t;

typedef enum
{
    RESISTOR_DISABLED,
    RESISTOR_ENABLED
} gpio_resistor_t;

typedef enum
{
    GPIO_OUTPUT,
    GPIO_INPUT
} gpio_dir_t;

/* For input this selects pull-up / pull-down if resistor enabled */
typedef enum
{
    GPIO_LOW,
    GPIO_HIGH
} gpio_output_t;

typedef enum
{
    GPIO_SEL_GPIO,
    GPIO_SEL_1,
    GPIO_SEL_2,
    GPIO_SEL_3
} gpio_selection_t;

typedef enum
{
    /* Port 1 */
    GPIO_LINE_DETECT_FRONT_RIGHT = GPIO_10,
    GPIO_UART_RXD = GPIO_11,
    GPIO_UART_TXD = GPIO_12,
    GPIO_LINE_DETECT_FRONT_LEFT = GPIO_13,
    GPIO_LINE_DETECT_BACK_LEFT = GPIO_14,
    GPIO_LINE_DETECT_BACK_RIGHT = GPIO_15,
    GPIO_I2C_SCL = GPIO_16,
    GPIO_I2C_SDA = GPIO_17,
    /* Port 2 */
    GPIO_IR_REMOTE = GPIO_20,
    GPIO_RANGE_SENSOR_FRONT_INT = GPIO_21,
    GPIO_XSHUT_FRONT = GPIO_22,
    GPIO_UNUSED_1 = GPIO_23,
    GPIO_MOTORS_LEFT_CC_2 = GPIO_24,
    GPIO_MOTORS_LEFT_CC_1 = GPIO_25,
    GPIO_TEST_LED = GPIO_26,
    GPIO_MOTORS_RIGHT_CC_2 = GPIO_27,
    /* Port 3 */
    GPIO_XSHUT_FRONT_RIGHT = GPIO_30,
    GPIO_XSHUT_RIGHT = GPIO_31,
    GPIO_XSHUT_FRONT_LEFT = GPIO_32,
    GPIO_XSHUT_LEFT = GPIO_33,
    GPIO_UNUSED_2 = GPIO_34,
    GPIO_PWM_MOTORS_LEFT = GPIO_35,
    GPIO_PWM_MOTORS_RIGHT = GPIO_36,
    GPIO_MOTORS_RIGHT_CC_1 = GPIO_37,
} gpio_t;

typedef struct
{
    gpio_t gpio;
    gpio_dir_t dir;
    gpio_output_t out;
    gpio_resistor_t resistor;
    gpio_selection_t selection;
} gpio_config_t;

typedef enum
{
    TRIGGER_RISING,
    TRIGGER_FALLING,
} trigger_t;

void gpio_init(void);
void gpio_configure(const gpio_config_t *config);
void gpio_set_direction(gpio_t gpio, gpio_dir_t direction);
void gpio_set_output(gpio_t gpio, gpio_output_t output);
bool gpio_get_input(gpio_t gpio);
void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor);
void gpio_set_selection(gpio_t gpio, gpio_selection_t selection);

void gpio_enable_interrupt(gpio_t gpio);
void gpio_disable_interrupt(gpio_t gpio);
void gpio_set_interrupt_trigger(gpio_t gpio, trigger_t trigger);
typedef void (*isr_function)(void);
void gpio_register_isr(gpio_t gpio, isr_function isr);

#endif /* GPIO_H */
