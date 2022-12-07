/*******************************************************
 * File Name    : hal_adc.c
 * Description  : hardware adc unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-23   |         oem
********************************************************/
#include "hal_adc.h"

int hal_adc_init(void)
{
    ADC_InitTypeDef ADCInitStruct;

    // TPH-TM
    GPIO_Config(HAL_ADC_IO_PORT, HAL_ADC_IO_PIN, ANALOG);
    ADCInitStruct.ADC_Channel = ADC_CHANNEL_6;
    ADCInitStruct.ADC_Mode = ADC_GPIO;
    ADC_Init(&ADCInitStruct);

    // ADC_GetVoltage(ADC_CHANNEL_2);
    return 0;
}
