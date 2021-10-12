#include "hw.h"
#include "state_machine.h"
#include "trace.h"
#include "test.h"
#include "led.h"
#include "motor.h"
#include "uart.h"
#include "ir_remote.h"
#include "gpio.h"

// TODO: How to handle motor circuit default in a good way? I mean what if sensor is in reset???
// Avoid potential power through motors in that case...
int main(void)
{
    hw_init();

    //uart_init();
    //TRACE_INFO("Booted");
    led_init();
    test_adc();
    while(1) {
        if (gpio_get_input(GPIO_LINE_DETECT_BACK_LEFT)) {
            led_set_enable(LED_TEST, true);
        } else {
            led_set_enable(LED_TEST, false);
        }
    }
    //state_machine_run();
}
