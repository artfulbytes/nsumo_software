#include "qre1113.h"
#include "gpio.h"
#include "adc.h"

void qre1113_init()
{
    adc_conf_t conf = { { false } };
    conf.enable[GPIO_PIN_IDX(GPIO_LINE_DETECT_FRONT_LEFT)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_LINE_DETECT_FRONT_RIGHT)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_LINE_DETECT_BACK_LEFT)] = true;
    conf.enable[GPIO_PIN_IDX(GPIO_LINE_DETECT_BACK_RIGHT)] = true;
    adc_init(&conf);
}

void qre1113_get_voltages(qre1113_voltages_t *voltages)
{
    adc_values_t adc_values = { 0 };
    adc_read(adc_values);
    voltages->front_left = adc_values[GPIO_PIN_IDX(GPIO_LINE_DETECT_FRONT_LEFT)];
    voltages->front_right = adc_values[GPIO_PIN_IDX(GPIO_LINE_DETECT_FRONT_RIGHT)];
    voltages->back_left = adc_values[GPIO_PIN_IDX(GPIO_LINE_DETECT_BACK_LEFT)];
    voltages->back_right = adc_values[GPIO_PIN_IDX(GPIO_LINE_DETECT_BACK_RIGHT)];
}
