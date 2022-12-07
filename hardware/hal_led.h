#ifndef _HAL_LED_H_
#define _HAL_LED_H_

#include "hal_config.h"

#define HAL_LED_PORT   GPIOC
#define HAL_LED_PIN    GPIO_Pin_7

#define HAL_LED_ON()   GPIO_SetBits(HAL_LED_PORT, HAL_LED_PIN);
#define HAL_LED_OFF()  GPIO_ResetBits(HAL_LED_PORT, HAL_LED_PIN);

int hal_led_init(void);

#endif
