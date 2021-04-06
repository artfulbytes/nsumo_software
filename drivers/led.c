#include "led.h"
#include "gpio.h"

static const gpio_config_t p1_0_led_config = {
    .gpio = GPIO_ADC_LEFT_SENSOR,
    .dir = GPIO_OUTPUT,
    .out = GPIO_HIGH,
    .resistor = RESISTOR_DISABLED,
    .selection = GPIO_SEL_GPIO
};

static const gpio_config_t p1_6_led_config = {
    .gpio = GPIO_ADC_RIGHT_SENSOR,
    .dir = GPIO_OUTPUT,
    .out = GPIO_HIGH,
    .resistor = RESISTOR_DISABLED,
    .selection = GPIO_SEL_GPIO
};

void led_init(led_t led)
{
    switch (led) {
    case LED_FIRST:
        gpio_configure(&p1_0_led_config);
        break;
    case LED_SECOND:
        gpio_configure(&p1_6_led_config);
        break;
    }
}

void led_set_mode(led_t led, led_mode_t mode)
{
    gpio_output_t output = (mode == LED_MODE_ON) ? GPIO_HIGH : GPIO_LOW;
    switch (led) {
    case LED_FIRST:
        gpio_set_output(GPIO_ADC_LEFT_SENSOR, output);
        break;
    case LED_SECOND:
        gpio_set_output(GPIO_ADC_RIGHT_SENSOR, output);
        break;
    }
}
