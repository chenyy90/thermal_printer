/*******************************************************
 * File Name    : hal_time.c
 * Description  : timer unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-12-06   |         oem
********************************************************/
#include "hal_time.h"

static hal_time_period_exec_func hal_time_period_exec = NULL;

int hal_time_period_execute_register(hal_time_period_exec_func exec_func)
{
    hal_time_period_exec = exec_func;
    return 0;
}

int hal_time_init(void)
{
    TIM_InitTypeDef TIM_InitStruct;

    /* Heat Ctrl Timer */
    TIM_InitStruct.period = USEC(500);
    TIM_InitStruct.TIMx = TIM6;
    TIM_Init(&TIM_InitStruct);
    TIM_ModeConfig(TIM6, TIM_Mode_TIMER);
    TIM_Cmd(TIM6, ENABLE);

    NVIC_EnableIRQ(TIM6_IRQn);
    NVIC_SetPriority(TIM6_IRQn, HAL_IRQ_TIMER_PRIORITY);
    return 0;
}

void TIMER6_IRQHandler(void)
{
    if (hal_time_period_exec)
    {
        hal_time_period_exec();
    }
}
