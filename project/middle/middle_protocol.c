/*******************************************************
 * File Name    : middle_protocol.c
 * Description  : printer protocol
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-12-10   |         oem
********************************************************/
#include "middle_protocol.h"
#include "app_esc.h"

static uint8_t middle_protocol_buff[MIDDLE_PROTOCOL_BUFF_SIZE] = {0};
static middle_protocol_struct middle_protocol;

#define middle_protocol_write_next() { \
    middle_protocol.write++; \
    if (middle_protocol.write >= MIDDLE_PROTOCOL_BUFF_SIZE) { \
        middle_protocol.write = 0; \
    } \
}

#define middle_protocol_read_next() { \
    middle_protocol.read++; \
    if (middle_protocol.read >= MIDDLE_PROTOCOL_BUFF_SIZE) { \
        middle_protocol.read = 0; \
    } \
}

int middle_protocol_buff_freesize(void)
{
    int freesize = 0;

    if (middle_protocol.write > middle_protocol.read)
    {
        freesize = middle_protocol.write - middle_protocol.read;
        freesize = MIDDLE_PROTOCOL_BUFF_SIZE - freesize;
    }
    else
    {
        freesize = middle_protocol.read - middle_protocol.write;
    }

    return freesize;
}

int middle_protocol_buff_empty(void)
{
    return (middle_protocol.read == middle_protocol.write);
}

int middle_protocol_buff_putch(uint8_t data)
{
    if (middle_protocol.flowctrl == 0 && middle_protocol_buff_freesize() <= MIDDLE_PROTOCOL_FLOWCTRL_SIZE)
    {
        /* 开启流控 */
        MIDDLE_PROTOCOL_FLOWCTRL_ON();
        middle_protocol.flowctrl = 1;
    }

    middle_protocol.buff[middle_protocol.write] = data;
    middle_protocol_write_next();
    return 0;
}

int middle_protocol_buff_getch(void)
{
    if (middle_protocol.flowctrl && middle_protocol_buff_freesize() > MIDDLE_PROTOCOL_FLOWCTRL_SIZE)
    {
        /* 关闭流控 */
        MIDDLE_PROTOCOL_FLOWCTRL_OFF();
        middle_protocol.flowctrl = 0;
    }

    if (middle_protocol.read == middle_protocol.write)
    {
        return -1;
    }

    uint8_t ch = middle_protocol.buff[middle_protocol.read];
    middle_protocol_read_next();
    return (int)ch;
}

int middle_protocol_progress(void)
{
    if (middle_protocol.flowctrl && middle_protocol_buff_freesize() > MIDDLE_PROTOCOL_FLOWCTRL_SIZE)
    {
        /* 关闭流控 */
        MIDDLE_PROTOCOL_FLOWCTRL_OFF();
        middle_protocol.flowctrl = 0;
    }

    if (middle_protocol.read == middle_protocol.write)
    {
        middle_protocol.state = 0;
        middle_protocol.timeout = MIDDLE_PROTOCOL_SYSTIME();
        return 0;
    }

    if (middle_protocol.state == 0)
    {
        /* 开始解析 */
        if (MIDDLE_PROTOCOL_SYSTIME() - middle_protocol.timeout >= 5000)
        {
            middle_protocol.state = 1;
        }
    }
    else
    {
        /* 解析 */
        int cmd_data = middle_protocol_buff_getch();
        if (cmd_data != -1)
        {
            app_esc_cmd_parse(cmd_data);
        }
    }

    return 0;
}

int middle_protocol_init(void)
{
    memset(&middle_protocol, 0, sizeof(middle_protocol));
    middle_protocol.buff = middle_protocol_buff;
    return 0;
}
