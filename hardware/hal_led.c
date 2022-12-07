#include "hal_led.h"

int hal_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = HAL_LED_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HAL_LED_PORT, &GPIO_InitStruct);

    HAL_LED_ON();
    return 0;
}
