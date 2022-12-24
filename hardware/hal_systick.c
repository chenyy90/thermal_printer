/*******************************************************
 * File Name    : hal_systick.c
 * Description  : system tick unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-23   |         oem
********************************************************/
#include "hal_systick.h"
#include "hal_config.h"

static volatile uint32_t hal_systick_time_count = 0;

uint32_t hal_systick_time(void)
{
    return hal_systick_time_count;
}

int hal_systick_delay(uint32_t t)
{
    uint32_t last_time = hal_systick_time_count;
    while(hal_systick_time_count - last_time < 1000);
    return 0;
}

int hal_systick_init(void)
{
    /* systick = 1ms */
    SysTick_Config(CPU_MHZ / 1000);
    return 0;
}

/**
  * @brief  Systick interrupt service function.
  * @param  None
  * @retval None
  */
void SYSTICK_IRQHandler(void)
{
    hal_systick_time_count++;
}
