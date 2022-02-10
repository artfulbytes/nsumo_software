#include "adc.h"
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

static volatile uint16_t data_transfer_block[ADC_CHANNEL_CNT] = { 0 };
static bool initialized = false;
static bool adc_channel_enabled[ADC_CHANNEL_CNT] = { false };
static volatile adc_values_t copied_adc_values = { 0 };
/* The DTO samples the channels contiguously, so this count will be larger than the
 * number of channels we actually use if they are not neighbouring channels. For example,
 * we may only use A0, A3, A4, but DTO still reads A1, A2. */
static uint8_t dtc_channel_cnt = 0;
static uint8_t last_chnl_idx = 0;

void adc_init(adc_conf_t *adc_conf)
{
    if (initialized) {
        return;
    }

    uint8_t adc10ae0 = 0;
    uint8_t first_chnl_idx = 0;
    for (int chnl = 0; chnl < ADC_CHANNEL_CNT; chnl++) {
        if (adc_conf->enable[chnl]) {
            adc10ae0 += (1 << chnl); // Enable corresponding GPIO pin
            adc_channel_enabled[chnl] = true;
            if (dtc_channel_cnt == 0) {
                first_chnl_idx = chnl;
            }
            last_chnl_idx = chnl;
            dtc_channel_cnt = (last_chnl_idx - first_chnl_idx) + 1;
        }
    }

    if (dtc_channel_cnt == 0) {
        return;
    }

    const uint16_t inch = last_chnl_idx * 4096;

    /* Configure it to run of ACLK (~8-32 kHz, depends on unit), to sample the channels
     * and then trigger an interrupt, which let us push the values to a buffer before
     * starting the sampling again. */
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + MSC + ADC10IE;
    ADC10CTL1 = inch + ADC10DIV_0 + CONSEQ_1 + SHS_0 + ADC10SSEL_1;
    ADC10AE0 = adc10ae0;
    ADC10DTC1 = dtc_channel_cnt;
    ADC10DTC0 = ADC10CT;
    ADC10SA = (uint16_t)data_transfer_block;
    ADC10CTL0 |= ENC + ADC10SC;
    initialized = true;
}

static uint32_t total_sample_cnt = 0;

void __attribute__((interrupt(ADC10_VECTOR))) adc_isr(void)
{
    for (int chnl = 0; chnl < ADC_CHANNEL_CNT; chnl++) {
        if (adc_channel_enabled[chnl]) {
            /* The ADC10 DTC writes the channel sample values in opposite order */
            copied_adc_values[chnl] = data_transfer_block[last_chnl_idx - chnl];
        }
    }
    ADC10CTL0 |= ENC + ADC10SC;
    total_sample_cnt++;
}

uint32_t adc_total_sample_cnt()
{
    __disable_interrupt();
    const uint32_t count = total_sample_cnt;
    __enable_interrupt();
    return count;
}

void adc_read(adc_values_t values)
{
    /* Disable global interrupt while retrieving the values.
     * If we only disable the ADC interrupt, the conversion and trigger
     * stops working after a while for unknown reason. */
    __disable_interrupt();
    for (int chnl = 0; chnl < ADC_CHANNEL_CNT; chnl++) {
        if (adc_channel_enabled[chnl]) {
            values[chnl] = copied_adc_values[chnl];
        }
    }
    __enable_interrupt();
}
