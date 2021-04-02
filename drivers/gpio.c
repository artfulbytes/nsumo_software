#include "gpio.h"
#include "stdint.h"

static volatile uint8_t* port_dir_registers[] =
{
    &P1DIR,
    &P2DIR
};
static volatile uint8_t* port_ren_registers[] =
{
    &P1REN,
    &P2REN
};
static volatile uint8_t* port_out_registers[] =
{
    &P1OUT,
    &P2OUT
};

static volatile uint8_t* port_sel_registers[] =
{
    &P1SEL,
    &P2SEL
};

// TODO: look into if this default configuring is dangerous
// I'm also getting "Detected uninitialized Port ..."
// Update: Output as default seems okay, and the warning/remark
// about uninitialized from CCS is wrong, it just can't detect the way I
// initialize my ports.
static const gpio_config_t gpio_initial_config[] =
{
    {GPIO_ADC_LEFT_SENSOR, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_ADC_FRONT_LEFT_SENSOR, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_ADC_FRONT_SENSOR, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_ADC_FRONT_RIGHT_SENSOR, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_ADC_RIGHT_SENSOR, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_P15, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_P16, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_P17, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},

    {GPIO_MOTORS_LEFT_CC_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_PWM_0, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_MOTORS_LEFT_CC_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_MOTORS_RIGHT_CC_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_PWM_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_MOTORS_RIGHT_CC_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_P26, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
    {GPIO_P27, GPIO_OUTPUT, GPIO_LOW, RESISTOR_DISABLED, GPIO_SEL_GPIO},
};

void gpio_init()
{
    int cfg_idx;
    for (cfg_idx = 0; cfg_idx < sizeof(gpio_initial_config); cfg_idx++) {
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
    switch (direction)
    {
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
    switch (output)
    {
    case GPIO_HIGH:
        *port_out_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    case GPIO_LOW:
        *port_out_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
        break;
    }
}

void gpio_set_resistor(gpio_t gpio, gpio_resistor_t resistor)
{
    switch (resistor)
    {
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
    switch (selection)
    {
    case GPIO_SEL_GPIO:
        *port_sel_registers[GPIO_PORT(gpio)] &= ~GPIO_PIN(gpio);
        break;
    case GPIO_SEL_1:
        *port_sel_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    }
}
