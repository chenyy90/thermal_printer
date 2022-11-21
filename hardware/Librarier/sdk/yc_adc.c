/*
File Name    : yc_adc.c
Author       : Yichip
Version      : V1.1
Date         : 2020/07/14
Description  : ADC encapsulation.
*/
#include "yc_adc.h"

#define ADC_RESULT (0x7FE&ADC_RDATA)

extern Boolean ipc_inited;
static int calculate_voltage(int result, int vol_high_ref, int vol_low_ref, int vol_high, int vol_low);

void ADC_Init(ADC_InitTypeDef *ADC_InitStruct)
{
    _ASSERT(IS_ADC_CHANNEL(ADC_InitStruct->ADC_Channel));
    _ASSERT(IS_ADC_MODE(ADC_InitStruct->ADC_Mode));
    /****************open dpll*******************/
    IpcInit();

    /****************config adc******************* */
    ADC_CTRL1 = 0x8f | ((ADC_InitStruct->ADC_Mode) << 4);
#if (VERSIONS == NO_BT)
    ADC_ENBLE = 0x7c;
#endif
    ADC_CTRL0 = 0x7f;
    ADC_CTRL2 = 0xaa;
    ADC_CTRL3 = 0x30 | ADC_InitStruct->ADC_Channel;
}

uint16_t  ADC_GetResult(ADC_ChxTypeDef ADC_Channel)
{
    if (ipc_inited == FALSE)	return 0;
    int nop =  WAIT_TIME;
#if (VERSIONS == NO_BT)
    ADC_ENBLE = 0x7c;
#endif
    ADC_CTRL0 = 0x7f;
    ADC_CTRL2 = 0xaa;
    ADC_CTRL3 = 0x30 | ADC_Channel;
    while (nop--);
    return	ADC_RESULT;
}

int ADC_Filt(ADC_ChxTypeDef ADC_Channel)
{
    int8_t i, j;
    uint16_t ADC_Buffer[FILT_NUM], tmp;
    int nop = WAIT_TIME;
    int cnt = 200;
    if (ipc_inited == FALSE)	return -1;
#if (VERSIONS == NO_BT)
    ADC_ENBLE = 0x7c;
#endif
    ADC_CTRL0 = 0x7f;
    ADC_CTRL2 = 0xaa;
    ADC_CTRL3 = 0x30 | ADC_Channel;

    while (nop--) {};

    if (REMOVE_NUM * 2 >= FILT_NUM)
    {
        return -1;
    }

    for (i = 0; i < FILT_NUM; i++)
    {
        ADC_Buffer[i] = ADC_RESULT;
        if (ADC_Buffer[i] != 0)
        {
            continue;
        }
        else
        {
            i--;
            if (0 == cnt--)
                return -1;
        }
#ifdef SDK_DEBUG
        MyPrintf("ADC_result,%d ", ADC_RESULT);
#endif
    }
    for (j = 1; j < FILT_NUM; j++)
    {
        tmp = ADC_Buffer[j];
        i = j - 1;
        while (i >= 0 && ADC_Buffer[i] > tmp)
        {
            ADC_Buffer[i + 1] = ADC_Buffer[i];
            i--;
        }
        ADC_Buffer[i + 1] = tmp;
    }

    for (tmp = 0, i = REMOVE_NUM; i < FILT_NUM - REMOVE_NUM; i++)
    {
        tmp += ADC_Buffer[i];
    }

    return tmp / (FILT_NUM - 2 * REMOVE_NUM);
}

int ADC_GetVoltage(ADC_ChxTypeDef ADC_Channel)
{
    int result = 0;
    int i = 0, j = 0;
    int voltage_low  = 2800;
    int voltage_high = 5000;
    int reg_voltage_low  = 640 * 1000;
    int ref_voltage_high  = 744 * 1000;
    int adc_result_buf[FILT_NUM] = {0};

    uint8_t getmode = (ADC_CTRL1 & 0x70) >> 4;
    uint8_t data[16] = {0};

    if (ipc_inited == FALSE)	return -1;

    //read otp reference
    read_otp(ADC_REF_OTP_ADDR, data, 16);

    switch (getmode)
    {
    case ADC_GPIO:
        reg_voltage_low  = ((data[3] << 8) | data[2]) * 1000;
        ref_voltage_high = ((data[9] << 8) | data[8]) * 1000;
        voltage_low = IO_LOW_VOLTAGE_REF;
        voltage_high = IO_HIHG_VOLTAGE_REF;
        break;

    case ADC_HVIN:
        reg_voltage_low  = ((data[5] << 8) | data[4]) * 1000; //OTP中3.3v校准值
        ref_voltage_high = ((data[11] << 8) | data[10]) * 1000; //OTP中5.0V校准值
        voltage_low = HVIN_LOW_VOLTAGE_REF;
        voltage_high = HVIN_HIHG_VOLTAGE_REF;
        break;

    case ADC_VINLPM:
        reg_voltage_low  = ((data[5] << 8) | data[4]) * 1000;
        ref_voltage_high = ((data[11] << 8) | data[10]) * 1000;
        voltage_low = VINLPM_LOW_VOLTAGE_REF;
        voltage_high = VINLPM_HIHG_VOLTAGE_REF;
        break;
    default:
        return -1;
    }

    if (reg_voltage_low == 0 || ref_voltage_high == 0)
        return -1;

    while (i < FILT_NUM)//get FFILT_NUM  result
    {
        result =  ADC_GetResult(ADC_Channel);
        if (result > 0)
        {
            adc_result_buf[i++] = calculate_voltage(result, ref_voltage_high, reg_voltage_low, voltage_high, voltage_low);
        }
        if (++j > FILT_NUM + 10)
            return -1;    //debug whiel(1)
    }

    for (j = 1; j < FILT_NUM; j++)
    {
        result = adc_result_buf[j];
        i = j - 1;
        while (i >= 0 && adc_result_buf[i] > result)
        {
            adc_result_buf[i + 1] = adc_result_buf[i];
            i--;
        }
        adc_result_buf[i + 1] = result;
    }

    for (result = 0, i = REMOVE_NUM; i < FILT_NUM - REMOVE_NUM; i++)
    {
        result += adc_result_buf[i];
    }
    return result / (FILT_NUM - 2 * REMOVE_NUM);
}

/**
  * @brief  calculate voltage ,acording reference value and reference voltage
  * @param  result       : adc result
  *         vol_high_ref : high voltage reference value
  *         vol_low_ref  : low voltage reference value
  *         vol_high     : high reference voltage (mV)
  *         vol_low      : low reference voltage
  * @retval voltage (mV)
  */
static int calculate_voltage(int result, int vol_high_ref, int vol_low_ref, int vol_high, int vol_low)
{
    int rega = result * 1000 - vol_low_ref;
    int regb = vol_high_ref - vol_low_ref;
    int regc = vol_high - vol_low;
    int temp = (rega * regc) / regb + vol_low;
    return temp;
}
