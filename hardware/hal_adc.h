#ifndef _HAL_ADC_H_
#define _HAL_ADC_H_

#include "hal_config.h"

//ADC管脚配置
#define HAL_ADC_IO_PORT  GPIOC
#define HAL_ADC_IO_PIN   GPIO_Pin_10

int hal_adc_init(void);

#endif
