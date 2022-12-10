#ifndef _MIDDLE_PROTOCOL_H_
#define _MIDDLE_PROTOCOL_H_

#include "middle_conf.h"
#include "hal_systick.h"

typedef struct
{
    uint8_t  state;
    uint8_t  flowctrl;
    uint16_t write;
    uint16_t read;
    uint8_t *buff;
    uint32_t timeout;
} middle_protocol_struct;

#define MIDDLE_PROTOCOL_BUFF_SIZE     (2048)
#define MIDDLE_PROTOCOL_FLOWCTRL_SIZE (256)
#define MIDDLE_PROTOCOL_SYSTIME()     hal_systick_time()
#define MIDDLE_PROTOCOL_FLOWCTRL_ON()
#define MIDDLE_PROTOCOL_FLOWCTRL_OFF()

int middle_protocol_buff_putch(uint8_t data);
int middle_protocol_buff_getch(void);
int middle_protocol_progress(void);
int middle_protocol_init(void);

#endif
