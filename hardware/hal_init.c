/*******************************************************
 * File Name    : hal_init.c
 * Description  : hardware init unit.
 -------------------------------------------------------
 Version         |      Date         |         Author
 -------------------------------------------------------
 1.0             |      2022-12-06   |         oem
********************************************************/
#include "hal_init.h"

int hal_init(void)
{
    /* System Clock: 96MHz*/
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);

    hal_systick_init();
    hal_log_init();
    hal_log_printf("halo printer ...\r\n");

    hal_led_init();
    hal_print_init();
    hal_adc_init();
    hal_button_init();
    hal_time_init();
    hal_bt_init();

    return 0;
}
