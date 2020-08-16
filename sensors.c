#include "sensors.h"
#include "adc.h"

static uint16_t adc_to_distance(uint16_t adc_value)
{
    // The GP2Y0A21YK0F produce a voltage between 0.4-2.4 V for
    // distance between 80-10 cm.
    // Since 3.3 V is used as reference and ADC is 10-bit the
    // value for 3.3 V is 1023.
    // 2.4 V is then 2.4*(1023/3.3)=744 and 0.4 V V is 0.4*(1023/3.3)=124

    if (adc_value < 124 || adc_value > 744)
    {
        return OUT_OF_RANGE;
    }

    // TODO: Adjust these values to my particular sensors
    // Scale linearly, ensure 124 equals 80 cm and 744 equals 10 cm
    return 80 - (adc_value-124) * 0.113;
}

void sensors_init()
{
    adc_init();
}

static adc_channel_values_t channel_values;
void sensors_get_distances(sensor_distances_t* distances)
{
    adc_read_channels(&channel_values);
    distances->left = adc_to_distance(channel_values.left_sensor);
    distances->front_left = adc_to_distance(channel_values.front_left_sensor);
    distances->front = adc_to_distance(channel_values.front_sensor);
    distances->front_right = adc_to_distance(channel_values.front_right_sensor);
    distances->right = adc_to_distance(channel_values.right_sensor);
}
