#include "hw.h"
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
// about uninitialized is wrong, it just can't detect the way I
// initialize my ports.
static const gpio_config_t gpio_initial_config[] =
{
    {GPIO_P10, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P11, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P12, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_MOTOR_FRONT_LEFT_CC_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_MOTOR_FRONT_LEFT_CC_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_MOTOR_BACK_LEFT_CC_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_MOTOR_BACK_LEFT_CC_2, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P17, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},

    {GPIO_P20, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_PWM_0, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P22, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P23, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_PWM_1, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P25, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P26, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
    {GPIO_P27, GPIO_OUTPUT, GPIO_LOW, RESISTOR_PULLDOWN, GPIO_SEL_GPIO},
};

static void gpio_set_initial_config()
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
    case RESISTOR_PULLDOWN:
        *port_ren_registers[GPIO_PORT(gpio)] |= GPIO_PIN(gpio);
        break;
    case RESISTOR_PULLUP:
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

static void hw_init_clocks()
{
    BCSCTL1 = CALBC1_1MHZ; // Basic Clock System Control 1 (MCLK?)
    DCOCTL = CALDCO_1MHZ;  // DCO Clock Frequency Control
}

static void hw_stop_watchdog_timer()
{
    // Stop this or the MSP430 will keep rebooting
    WDTCTL = WDTPW | WDTHOLD;
}

void hw_init()
{
    hw_stop_watchdog_timer();
    hw_init_clocks();
    gpio_set_initial_config();
}
