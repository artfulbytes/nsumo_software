#ifndef __hw
#define __hw

#include <msp430.h>

// TODO: Using 16-bit atm, but this should be possible
// with 8-bit too. But then I need to convert the bit position to
// decimal somehow.
#define GPIO_PORT_OFFSET     (8u)
#define GPIO_PORT_MASK       (0xFFu << GPIO_PORT_OFFSET)
#define GPIO_PORT(gpio)      ((gpio & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
#define GPIO_PIN_MASK        (0xFFu)
#define GPIO_PIN(gpio)       (gpio & GPIO_PIN_MASK)
#define GPIO_MAKE(port, pin) ((port << GPIO_PORT_OFFSET) | pin)

typedef enum
{
    GPIO_PORT_1,
    GPIO_PORT_2
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

typedef enum
{
    GPIO_LOW,
    GPIO_HIGH
} gpio_output_t;

typedef enum
{
    GPIO_SEL_GPIO,
    GPIO_SEL_1 // ?
} gpio_selection_t;

typedef enum
{
    // Port 1
    GPIO_P10 = GPIO_MAKE(GPIO_PORT_1, BIT0),
    GPIO_P11 = GPIO_MAKE(GPIO_PORT_1, BIT1),
    GPIO_P12 = GPIO_MAKE(GPIO_PORT_1, BIT2), // 1.2 used for usb transfer...
    GPIO_P13 = GPIO_MAKE(GPIO_PORT_1, BIT3),
    GPIO_P14 = GPIO_MAKE(GPIO_PORT_1, BIT4),
    GPIO_P15 = GPIO_MAKE(GPIO_PORT_1, BIT5),
    GPIO_P16 = GPIO_MAKE(GPIO_PORT_1, BIT6),
    GPIO_P17 = GPIO_MAKE(GPIO_PORT_1, BIT7),
    // Port 2
    GPIO_MOTORS_LEFT_CC_1 = GPIO_MAKE(GPIO_PORT_2, BIT0),
    GPIO_PWM_0 = GPIO_MAKE(GPIO_PORT_2, BIT1),
    GPIO_MOTORS_LEFT_CC_2 = GPIO_MAKE(GPIO_PORT_2, BIT2),
    GPIO_MOTORS_RIGHT_CC_1 = GPIO_MAKE(GPIO_PORT_2, BIT3),
    GPIO_PWM_1 = GPIO_MAKE(GPIO_PORT_2, BIT4),
    GPIO_MOTORS_RIGHT_CC_2 = GPIO_MAKE(GPIO_PORT_2, BIT5),
    GPIO_P26 = GPIO_MAKE(GPIO_PORT_2, BIT6),
    GPIO_P27 = GPIO_MAKE(GPIO_PORT_2, BIT7)
} gpio_t;

typedef struct
{
    gpio_t gpio;
    gpio_dir_t dir;
    gpio_output_t out;
    gpio_resistor_t resistor;
    gpio_selection_t selection;
} gpio_config_t;

void gpio_configure(const gpio_config_t* config);
void gpio_set_direction(gpio_t gpio, gpio_dir_t direction);
void gpio_set_output(gpio_t gpio, gpio_output_t output);
void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor);
void gpio_set_selection(gpio_t gpio, gpio_selection_t selection);
void hw_init();

#endif //__hw
