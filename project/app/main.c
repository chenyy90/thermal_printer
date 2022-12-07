#include "hal_init.h"
#include "hal_log.h"
#include "middle_mmu.h"
#include "middle_print.h"
#include "middle_print_list.h"

static int period_execute(void)
{
    hal_button_progress();
    return 0;
}

int main(void)
{
    hal_init();
    hal_time_period_execute_register(period_execute);

    middle_mmu_init();
    middle_print_init();
    HAL_LOGD("MIAN", "main while ...\r\n");

    while(1)
    {
        if (hal_button_value())
        {
            HAL_LOGD("MIAN", "keypress ...\r\n");

            uint8_t testdata[MIDDLE_PRINT_DATA_SIZE];
            for (int byte=0; byte<72; byte++)
            {
                testdata[byte] = 0xff;
            }
            middle_print_list_put(testdata, MIDDLE_PRINT_DATA_SIZE);
        }

        middle_print_progress();
    }
}
