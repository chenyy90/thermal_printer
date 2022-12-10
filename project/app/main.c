/*******************************************************
 * File Name    : main.c
 * Description  : main function
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-10   |         oem
********************************************************/
#include "hal_init.h"
#include "hal_log.h"
#include "middle_mmu.h"
#include "middle_print.h"
#include "middle_print_list.h"
#include "middle_protocol.h"

static int period_execute(void)
{
    hal_button_progress();
    return 0;
}

static uint8_t testdata[MIDDLE_PRINT_DATA_SIZE];
static uint16_t testline = 0;

int main(void)
{
    hal_init();
    hal_time_period_execute_register(period_execute);

    middle_mmu_init();
    middle_print_init();
    middle_protocol_init();
    HAL_LOGD("MIAN", "main while ...\r\n");

    while(1)
    {
        if (hal_button_value())
        {
            HAL_LOGD("MIAN", "keypress ...\r\n");
            testline = 1;
        }

        if (testline && testline < 10)
        {
            for (int byte=0; byte<72; byte++)
            {
                testdata[byte] = 0xff;
            }

            if (middle_print_list_put(testdata, MIDDLE_PRINT_DATA_SIZE) == 0)
            {
                testline++;
            }
        }
        else
        {
            testline = 0;
        }

        middle_print_progress();
        middle_protocol_progress();
    }
}
