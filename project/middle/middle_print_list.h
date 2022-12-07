#ifndef _MIDDLE_PRINT_LIST_H_
#define _MIDDLE_PRINT_LIST_H_

#include "middle_conf.h"

typedef struct
{
    uint16_t segment_number;                           // 加热分段数
    uint16_t segment_ave_dot;                          // 加热分段点数
    uint8_t  *segment_data[MIDDLE_PRINT_HEAT_SEG_MAX]; // 分段打印数据
    uint32_t heat_time;                                // 加热时间
    uint32_t motor_speed_time;                         // 当前电机速度
} middle_print_list_struct;

int middle_print_list_is_free(void);
int middle_print_list_is_empty(void);
int middle_print_list_put(uint8_t *line_data, uint16_t line_datalen);
middle_print_list_struct *middle_print_list_get(void);
int middle_print_list_destory(middle_print_list_struct *p);
int middle_print_list_init(void);

#endif
