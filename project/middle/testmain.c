#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "middle_mmu.h"
#include "middle_print_list.h"

uint8_t testdata[MIDDLE_PRINT_DATA_SIZE] = {0};

int main(int argc, char **argv)
{
    middle_print_list_struct *list_get;

    middle_mmu_init();
    middle_print_list_init();

    for (int byte=0; byte<70; byte++)
    {
        testdata[byte] = 0xff;
    }

    middle_print_list_put(testdata, MIDDLE_PRINT_DATA_SIZE);

    list_get = middle_print_list_get();
    if (list_get)
    {
        printf("listget : heattime %d, motor %d, segment %d, ave %d\r\n",
            list_get->heat_time, list_get->motor_speed_time,
            list_get->segment_number, list_get->segment_ave_dot);
        for (int i=0; i<list_get->segment_number; i++)
        {
            for (int byte=0; byte<MIDDLE_PRINT_DATA_SIZE; byte++)
            {
                printf("%02x, ", list_get->segment_data[i][byte]);
            }
            printf("\r\n");
            middle_mmu_free(list_get->segment_data[i]);
        }
    }

    return 0;
}
