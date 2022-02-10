#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    ADC_CHANNEL_0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
    ADC_CHANNEL_CNT
} adc_channel_t;

typedef struct adc_conf
{
    bool enable[ADC_CHANNEL_CNT];
} adc_conf_t;

typedef uint16_t adc_values_t[ADC_CHANNEL_CNT];

void adc_init(adc_conf_t *conf);
void adc_read(adc_values_t values);
uint32_t adc_total_sample_cnt(void);

#endif /* ADC_H */
