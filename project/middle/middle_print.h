#ifndef _MIDDLE_PRINT_H_
#define _MIDDLE_PRINT_H_

#include "middle_conf.h"

typedef enum
{
    MIDDLE_PRT_STA_IDLE = 0,
    MIDDLE_PRT_STA_LATCH,
    MIDDLE_PRT_STA_LATCH_TO_STROBE,
    MIDDLE_PRT_STA_STROBE,
    MIDDLE_PRT_STA_WAIT,
} middle_print_state_enum;

typedef enum
{
    MIDDLE_PRT_MTR_STA_IDLE = 0,
    MIDDLE_PRT_MTR_STA_FEED,
} middle_print_motor_state_enum;

typedef struct
{
    uint8_t  print_sta;              /* 打印状态 */  
    uint8_t  motor_sta;              /* 电机状态 */
    uint8_t  motor_step;             /* 电机步数 */
    uint16_t slice_number;           /* 分段加热数 */
    uint16_t slice_total;            /* 分段加热总数 */
    uint16_t slice_bits;             /* 分段加热点数 */
    uint32_t motor_step_move;        /* 电机移动步数 */ 
    uint32_t motor_step_total;       /* 电机总步数 */ 
    uint32_t motor_step_target;      /* 电机预估行走步数 */
    uint32_t slice_heat_time;        /* 分段加热时间 */
    uint32_t motor_speed_time_last;  /* 电机最新行进时间 */
    uint32_t motor_speed_time;       /* 电机行进时间 */
} middle_print_manager_struct;

int middle_print_motor_spwm_handle(void);
int middle_print_latch_to_heat_handle(void);
int middle_print_heat_haddle(void);

int middle_print_motor_linefeed_step(void);
int middle_print_is_busy(void);
int middle_print_progress(void);
int middle_print_init(void);

#endif
