/*******************************************************
 * File Name    : hal_button.c
 * Description  : hardware button unit.
 -------------------------------------------------------
 Version         |      Date         |         Author
 -------------------------------------------------------
 1.0             |      2022-12-06   |         oem
********************************************************/
#include "hal_button.h"

static int hal_button_clicked_timeout = 0;
static int hal_button_clicked_value = 0;

int hal_button_progress(void)
{
    if (hal_button_clicked_timeout)
    {
        hal_button_clicked_timeout++;
        if (hal_button_clicked_timeout > 100)
        {
            hal_button_clicked_timeout = 0;

            if (Power_Keyscan())
            {
                hal_button_clicked_value = 1;
            }
        }
    }
    else if (Power_Keyscan())
    {
        hal_button_clicked_timeout = 1;
    }

    return 0;
}

int hal_button_value(void)
{
    int vaule = hal_button_clicked_value;
    hal_button_clicked_value = 0;
    return vaule;
}

int hal_button_init(void)
{
    return 0;
}
