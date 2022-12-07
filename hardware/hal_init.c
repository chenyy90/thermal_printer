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
    hal_systick_init();
    hal_log_init();
    hal_log_printf("halo printer ...\r\n");

    hal_led_init();
    hal_print_init();
    hal_adc_init();
    hal_button_init();
    hal_time_init();

    return 0;
}
