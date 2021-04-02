#ifndef ADC_H
#define ADC_H

#include <stdint.h>

typedef struct {
    uint16_t left_sensor;
    uint16_t front_left_sensor;
    uint16_t front_sensor;
    uint16_t front_right_sensor;
    uint16_t right_sensor;
} adc_channel_values_t;

void adc_init(void);
void adc_read_channels(adc_channel_values_t* channels);

#endif /* ADC_H */
