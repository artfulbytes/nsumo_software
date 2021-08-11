#include "adc.h"
#include "gpio.h"

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <defines.h>

/* We only need five channels, but the DTO samples them contiguously and the pin of
 * channel 1 and 2 is occupied by other functions. */
#define NUM_ADC_CHANNELS 7

typedef enum
{
    /* The ADC10 DTC writes the channel sample values in opposite order */
    CHANNEL_SENSOR_LEFT = 6,
    CHANNEL_UNUSED_0 = 5, /* Occupied by UART */
    CHANNEL_UNUSED_1 = 4, /* Occupied by UART */
    CHANNEL_SENSOR_FRONT_LEFT = 3,
    CHANNEL_SENSOR_FRONT = 2,
    CHANNEL_SENSOR_FRONT_RIGHT = 1,
    CHANNEL_SENSOR_RIGHT = 0
} channel_t;

static volatile uint16_t data_transfer_block[NUM_ADC_CHANNELS];
static volatile uint16_t samples_buffer_left_sensor[3];
static volatile uint16_t samples_buffer_front_left_sensor[3];
static volatile uint16_t samples_buffer_front_sensor[3];
static volatile uint16_t samples_buffer_front_right_sensor[3];
static volatile uint16_t samples_buffer_right_sensor[3];
static volatile uint16_t sample_idx = 0;
static bool initialized = false;

static uint16_t median_filter_3(uint16_t first, uint16_t second, uint16_t third)
{
    uint16_t middle;

    if ((first <= second) && (first <= third))
    {
        middle = (second <= third) ? second : third;
    }
    else if ((second <= first) && (second <= third))
    {
        middle = (first <= third) ? first : third;
    }
    else
    {
        middle = (first <= second) ? first : second;
    }
    return middle;
}

void adc_init()
{
    if (initialized) {
        return;
    }

    /* Configure it to run of ACLK (~8-32 kHz, depends on unit). The exact speed doesn't matter,
     * as long as we sample faster than ~40 ms, we should be fine, because that's the update rate
     * of the range sensor. Moreover, we configure it to sample the channels and then trigger
     * an interrupt, which let us push the values to a buffer before starting the sampling
     * again. */
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + MSC + ADC10IE;
    ADC10CTL1 = INCH_6 + ADC10DIV_0 + CONSEQ_1 + SHS_0 + ADC10SSEL_1;
    ADC10AE0 = GPIO_PIN(GPIO_ADC_LEFT_SENSOR) +
               GPIO_PIN(GPIO_ADC_FRONT_LEFT_SENSOR) +
               GPIO_PIN(GPIO_ADC_FRONT_SENSOR) +
               GPIO_PIN(GPIO_ADC_FRONT_RIGHT_SENSOR) +
               GPIO_PIN(GPIO_ADC_RIGHT_SENSOR);
    ADC10DTC1 = NUM_ADC_CHANNELS;
    ADC10DTC0 = ADC10CT;
    ADC10SA = (uint16_t)data_transfer_block;
    ADC10CTL0 |= ENC + ADC10SC;
    initialized = true;
}
// TODO Remove extra line below... (look at ti examples on how they write a single line instead)
void adc_isr(void) __attribute__ ((interrupt (ADC10_VECTOR)));
void adc_isr(void)
{
    ADC10CTL0 &= ~ENC;
    sample_idx += 1;
    if (sample_idx == ARRAY_SIZE(samples_buffer_left_sensor)) {
        sample_idx = 0;
    }

    samples_buffer_left_sensor[sample_idx] = data_transfer_block[CHANNEL_SENSOR_LEFT];
    samples_buffer_front_left_sensor[sample_idx] = data_transfer_block[CHANNEL_SENSOR_FRONT_LEFT];
    samples_buffer_front_sensor[sample_idx] = data_transfer_block[CHANNEL_SENSOR_FRONT];
    samples_buffer_front_right_sensor[sample_idx] = data_transfer_block[CHANNEL_SENSOR_FRONT_RIGHT];
    samples_buffer_right_sensor[sample_idx] = data_transfer_block[CHANNEL_SENSOR_RIGHT];

    ADC10CTL0 |= ENC + ADC10SC;
}

void adc_read(adc_channel_values_t* channel_values)
{
    /* Disable ADC interrupt while retrieving the values */
    ADC10CTL0 &= ~ADC10IE;
    /* Median filter the ADC values to remove spikes. This might also get better
     * once we add a capacitor on the range sensor. */
    channel_values->left_sensor = median_filter_3(samples_buffer_left_sensor[0],
                                                  samples_buffer_left_sensor[1],
                                                  samples_buffer_left_sensor[2]);
    channel_values->front_left_sensor = median_filter_3(samples_buffer_front_left_sensor[0],
                                                        samples_buffer_front_left_sensor[1],
                                                        samples_buffer_front_left_sensor[2]);
    channel_values->front_sensor = median_filter_3(samples_buffer_front_sensor[0],
                                                   samples_buffer_front_sensor[1],
                                                   samples_buffer_front_sensor[2]);
    channel_values->front_right_sensor = median_filter_3(samples_buffer_front_right_sensor[0],
                                                         samples_buffer_front_right_sensor[1],
                                                         samples_buffer_front_right_sensor[2]);
    channel_values->right_sensor = median_filter_3(samples_buffer_right_sensor[0],
                                                   samples_buffer_right_sensor[1],
                                                   samples_buffer_right_sensor[2]);
    ADC10CTL0 |= ADC10IE;
    ADC10CTL0 |= ENC + ADC10SC;
}
