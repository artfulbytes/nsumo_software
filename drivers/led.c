#include "led.h"
#include "gpio.h"

static const gpio_config_t p26_led_config = { .gpio = GPIO_TEST_LED,
                                              .dir = GPIO_OUTPUT,
                                              .out = GPIO_HIGH,
                                              .resistor = RESISTOR_DISABLED,
                                              .selection = GPIO_SEL_GPIO };

void led_init() { gpio_configure(&p26_led_config); }

void led_set_enable(led_t led, bool enable)
{
    gpio_output_t output = enable ? GPIO_HIGH : GPIO_LOW;
    switch (led) {
    case LED_TEST:
        gpio_set_output(GPIO_TEST_LED, output);
        break;
    }
}
