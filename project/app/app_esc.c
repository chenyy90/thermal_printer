/*******************************************************
 * File Name    : app_esc.c
 * Description  : epson cmd support
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-10   |         oem
********************************************************/
#include "app_esc.h"
#include "hal_log.h"
#include "middle_protocol.h"

int app_esc_cmd_parse(uint8_t cmd)
{
    if (cmd == 0x1B)
    {
        /* 打印支持 */
        switch(cmd)
        {
            case 0x2A:{

                }break;
            case 0x40:{
                if (middle_protocol_buff_getch() != 0x12 || middle_protocol_buff_getch() != 0x23)
                {
                    return -1;
                }

                uint8_t density = middle_protocol_buff_getch();
                HAL_LOGD("APP ESC", "set density %d\r\n", density);
                }break;
            case 0x64:{
                uint8_t feedline = middle_protocol_buff_getch();
                HAL_LOGD("APP ESC", "set feedline %d\r\n", feedline);
                }break;
            case 0x1D:{

                }break;
            default:break;
        }
    }
    else if (cmd == 0xFF)
    {
        /* 产测，升级 */
    }

    return 0;
}
