#include "adc.h"
#include "gpio.h"

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_ADC_CHANNELS 5

typedef enum
{
    // ADC10 stores the result in opposite order in the sample array
    CHANNEL_SENSOR_LEFT = 4,
    CHANNEL_SENSOR_FRONT_LEFT = 3,
    CHANNEL_SENSOR_FRONT = 2,
    CHANNEL_SENSOR_FRONT_RIGHT = 1,
    CHANNEL_SENSOR_RIGHT = 0
} channel_t;

static volatile uint16_t samples[NUM_ADC_CHANNELS];
static bool initialized = false;

void adc_init()
{
    if (initialized) {
        return;
    }
    // SREF_0: VCC (3.3v) and GND (0v) as reference
    // ADC10SHT_2: sample and hold time for 16 x ADC10CLK
    // ADC10ON: Enable ADC10
    // MSC: Multi sample and convert, must set this to automatically sample each channel in CONSEQ_1
    ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10ON + MSC;
    // INCH_4: Start with channel 4 and count downwards
    // ADC10DIV_0: No division of selected clock source
    // CONSEQ_1: Sample and convert sequence of channels once
    // SHS_0: Trigger sampling with ADC10SC pin
    // ADC10SSEL_0: ADC10OSC clock source
    ADC10CTL1 = INCH_4 + ADC10DIV_0 + CONSEQ_1 + SHS_0 + ADC10SSEL_0;
    // Enable sampling from p1.0, p1.1, ... p1.4
    // TODO: Use values of GPIO_PIN() here instead,
    ADC10AE0 = GPIO_PIN(GPIO_ADC_LEFT_SENSOR) +
               GPIO_PIN(GPIO_ADC_FRONT_LEFT_SENSOR) +
               GPIO_PIN(GPIO_ADC_FRONT_SENSOR) +
               GPIO_PIN(GPIO_ADC_FRONT_RIGHT_SENSOR) +
               GPIO_PIN(GPIO_ADC_RIGHT_SENSOR);
    // Number of transfers per block
    ADC10DTC1 = NUM_ADC_CHANNELS;
    // Continuous transfer
    ADC10DTC0 = ADC10CT;
    // Set address to save samples to
    ADC10SA = (uint16_t)samples;
    initialized = true;
}

void adc_read_channels(adc_channel_values_t* channel_values)
{
    // Start sampling and conversion
    ADC10CTL0 |= ENC + ADC10SC;

    // TODO: low power mode and wake on interrupt here instead.
    // Wait for sampling and conversion to finish
    while (ADC10CTL1 & ADC10BUSY);

    channel_values->left_sensor = samples[CHANNEL_SENSOR_LEFT];
    channel_values->front_left_sensor = samples[CHANNEL_SENSOR_FRONT_LEFT];
    channel_values->front_sensor = samples[CHANNEL_SENSOR_FRONT];
    channel_values->front_right_sensor = samples[CHANNEL_SENSOR_FRONT_RIGHT];
    channel_values->right_sensor = samples[CHANNEL_SENSOR_RIGHT];
}

