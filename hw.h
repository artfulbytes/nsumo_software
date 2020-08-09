#ifndef HW_C_
#define HW_C_

#include <msp430.h>

#define GPIO_PORT_OFFSET     (4u)
#define GPIO_PORT_MASK       (0xFFu << GPIO_PORT_OFFSET)
#define GPIO_PORT(gpio)      ((gpio & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
#define GPIO_PIN_MASK        (0x000Fu)
#define GPIO_PIN(gpio)       (gpio & GPIO_PIN_MASK)
#define GPIO_MAKE(port, pin) ((port << GPIO_PORT_OFFSET) | pin)

typedef enum
{
    GPIO_PORT_1,
    GPIO_PORT_2
} gpio_port_t;

typedef enum
{
    RESISTOR_PULLUP,
    RESISTOR_PULLDOWN
} gpio_resistor_t;

typedef enum
{
    GPIO_OUTPUT,
    GPIO_INPUT
} gpio_dir_t;

typedef enum
{
    GPIO_LOW,
    GPIO_HIGH
} gpio_output_t;

typedef enum
{
    GPIO_SEL_0,
    GPIO_SEL_1
} gpio_selection_t;

typedef enum
{
    // Port 1
    GPIO_P10 = GPIO_MAKE(GPIO_PORT_1, BIT0),
    GPIO_P11 = GPIO_MAKE(GPIO_PORT_1, BIT1),
    GPIO_P12 = GPIO_MAKE(GPIO_PORT_1, BIT2), // 1.2 used for usb transfer...
    GPIO_MOTOR_FRONT_LEFT_CC_1 = GPIO_MAKE(GPIO_PORT_1, BIT3),
    GPIO_MOTOR_FRONT_LEFT_CC_2 = GPIO_MAKE(GPIO_PORT_1, BIT4),
    GPIO_MOTOR_FRONT_RIGHT_CC_1 = GPIO_MAKE(GPIO_PORT_1, BIT5),
    GPIO_MOTOR_FRONT_RIGHT_CC_2 = GPIO_MAKE(GPIO_PORT_1, BIT6),
    GPIO_P17 = GPIO_MAKE(GPIO_PORT_1, BIT7),
    // Port 2
    GPIO_P20 = GPIO_MAKE(GPIO_PORT_2, BIT0),
    GPIO_PWM_0 = GPIO_MAKE(GPIO_PORT_2, BIT1),
    GPIO_P22 = GPIO_MAKE(GPIO_PORT_2, BIT2),
    GPIO_P23 = GPIO_MAKE(GPIO_PORT_2, BIT3),
    GPIO_PWM_1 = GPIO_MAKE(GPIO_PORT_2, BIT4),
    GPIO_P25 = GPIO_MAKE(GPIO_PORT_2, BIT5),
    GPIO_P26 = GPIO_MAKE(GPIO_PORT_2, BIT6),
    GPIO_P27 = GPIO_MAKE(GPIO_PORT_2, BIT7)
} gpio_t;

typedef struct
{
    gpio_t gpio;
    gpio_dir_t dir;
    gpio_output_t out;
    gpio_resistor_t resistor;
} gpio_config_t;

void gpio_configure(const gpio_config_t* config);
void gpio_set_direction(gpio_t gpio, gpio_dir_t direction);
void gpio_set_output(gpio_t gpio, gpio_output_t output);
void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor);
void gpio_set_selection(gpio_t gpio, gpio_selection_t selection);
void hw_init();

#endif /* HW_C_ */
