/*******************************************************
 * File Name    : middle_print.c
 * Description  : printer print ctrl
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-02   |         oem
********************************************************/
#include "middle_print.h"
#include "middle_print_list.h"
#include "hal_print.h"
#include "hal_log.h"

static middle_print_manager_struct middle_print_mgr;
static middle_print_list_struct *middle_print_list;
static uint8_t middle_print_line_data[MIDDLE_PRINT_DATA_SIZE] = {0};

#define MOTOR_SPWM_LEN        (18)
#define MOTOR_SPWM_PHASE1     (1 * MOTOR_SPWM_LEN)
#define MOTOR_SPWM_PHASE2     (2 * MOTOR_SPWM_LEN)
#define MOTOR_SPWM_PHASE3     (3 * MOTOR_SPWM_LEN)
#define MOTOR_SPWM_PHASE4     (4 * MOTOR_SPWM_LEN)
#define MOTOR_LINEFEED_STEP   (MOTOR_SPWM_LEN * 2)   /* 300DPI:走纸1行2步; 200DPI:3步 */

// const uint16_t motor_spwm_table[MOTOR_SPWM_LEN + 1] = {
//     0, 20, 40, 60, 80, 98, 116, 134, 150, 165, 179, 191, 202, 212, 219, 226, 230, 233, 234,
// };
const uint16_t motor_spwm_table[MOTOR_SPWM_LEN + 1] = {
 292, 308, 324, 339, 345, 353, 367, 375, 380, 393, 415, 420, 425, 430, 435, 439, 443, 447, 451
};

static int middle_print_stop(void);

// 电机驱动处理
int middle_print_motor_spwm_handle(void)
{
    if (middle_print_mgr.motor_step_total >= middle_print_mgr.motor_step_target)
    {
        /* 打印步数超限制 */
        hal_print_motor_stop();
        return 0;
    }
    else if (middle_print_mgr.motor_speed_time_last != middle_print_mgr.motor_speed_time)
    {
        hal_print_motor_speed_time(middle_print_mgr.motor_speed_time);
        middle_print_mgr.motor_speed_time_last = middle_print_mgr.motor_speed_time;
    }

    if (middle_print_mgr.motor_step >= MOTOR_SPWM_PHASE4)
    {
        middle_print_mgr.motor_step = 0;
    }

    uint8_t step = middle_print_mgr.motor_step + 1;

    // A1B1 -> B1A2 -> A2B2 -> B2A1
    if (step <= MOTOR_SPWM_PHASE1)
    {
        HAL_PRT_MOTOR_A1(motor_spwm_table[MOTOR_SPWM_PHASE1 - step + 1]);
        HAL_PRT_MOTOR_B1(motor_spwm_table[step]);
        HAL_PRT_MOTOR_A2(0);
        HAL_PRT_MOTOR_B2(0);
    }
    else if (step <= MOTOR_SPWM_PHASE2)
    {
        HAL_PRT_MOTOR_B1(motor_spwm_table[MOTOR_SPWM_PHASE2 - step]);
        HAL_PRT_MOTOR_A2(motor_spwm_table[step - MOTOR_SPWM_PHASE1 - 1]);
        HAL_PRT_MOTOR_A1(0);
        HAL_PRT_MOTOR_B2(0);
    }
    else if (step <= MOTOR_SPWM_PHASE3)
    {
        HAL_PRT_MOTOR_A2(motor_spwm_table[MOTOR_SPWM_PHASE3 - step + 1]);
        HAL_PRT_MOTOR_B2(motor_spwm_table[step - MOTOR_SPWM_PHASE2]);
        HAL_PRT_MOTOR_A1(0);
        HAL_PRT_MOTOR_B1(0);
    }
    else if (step <= MOTOR_SPWM_PHASE4)
    {
        HAL_PRT_MOTOR_B2(motor_spwm_table[MOTOR_SPWM_PHASE4 - step]);
        HAL_PRT_MOTOR_A1(motor_spwm_table[step - MOTOR_SPWM_PHASE3 - 1]);
        HAL_PRT_MOTOR_A2(0);
        HAL_PRT_MOTOR_B1(0);
    }

    middle_print_mgr.motor_step_total++;
    middle_print_mgr.motor_step = step;
    return 0;
}

// 判断电机是否走行完毕
static int middle_print_motor_linefeed_completed(void)
{
    if ((middle_print_mgr.motor_step_total - middle_print_mgr.motor_step_move) >= MOTOR_LINEFEED_STEP)
    {
        middle_print_mgr.motor_step_move = middle_print_mgr.motor_step_total;
        return 1;
    }
    return 0;
}

// 打印数据装载
static int middle_print_line_data_latch(void)
{
    memcpy(&middle_print_line_data,
            &middle_print_list->segment_data[middle_print_mgr.slice_number][0],
            MIDDLE_PRINT_DATA_SIZE);
    hal_print_send_data(middle_print_line_data, MIDDLE_PRINT_DATA_SIZE);
    return 0;
}

// 数据转载完毕，开始加热
int middle_print_latch_to_heat_handle(void)
{
    /* 数据传输完毕，进行数据装载 */
    HAL_PRT_NLAT_LOW();
    middle_print_mgr.print_sta = MIDDLE_PRT_STA_LATCH;
    hal_print_heat_time(5);
    return 0;
}

// 加热控制
int middle_print_heat_haddle(void)
{
    if (middle_print_mgr.print_sta == MIDDLE_PRT_STA_LATCH)
    {
        /* 数据装载完毕 */
        HAL_PRT_NLAT_HIGH();
        middle_print_mgr.print_sta = MIDDLE_PRT_STA_LATCH_TO_STROBE;
        hal_print_heat_time(10);
    }
    else if (middle_print_mgr.print_sta == MIDDLE_PRT_STA_LATCH_TO_STROBE)
    {
        /* 开始加热 */
        HAL_PRT_STB_HIGH();
        middle_print_mgr.print_sta = MIDDLE_PRT_STA_STROBE;
        hal_print_heat_time(middle_print_mgr.slice_heat_time);
    }
    else if (middle_print_mgr.print_sta == MIDDLE_PRT_STA_STROBE)
    {
        /* 结束本分段加热 */
        HAL_PRT_STB_LOW();

        middle_print_mgr.slice_number++;
        if (middle_print_mgr.slice_number < middle_print_mgr.slice_total)
        {
            /* 当前行还未加热完毕，紧接着下一分段加热 */
            middle_print_line_data_latch();
        }
        else
        {
            middle_print_mgr.slice_number = 0;
            middle_print_mgr.print_sta = MIDDLE_PRT_STA_WAIT;

            /* 当前行都打印完毕, 回收打印数据空间，等待马达走行完成 */
            middle_print_list_destory(middle_print_list);
            middle_print_list = NULL;

            /* 取出下一行数据 */
            middle_print_list = middle_print_list_get();
            if (middle_print_list == NULL)
            {
                /* 没有下一行数据，停止打印 */
                middle_print_stop();
            }
            else
            {
                /* 加热时间 */
                middle_print_mgr.slice_heat_time = middle_print_list->heat_time;

                if (middle_print_motor_linefeed_completed())
                {
                    /* 马达行进一行完成，开始进行下一行数据进行打印 */
                    middle_print_line_data_latch();
                }
                else
                {
                    /* 等待走纸完成 */
                    hal_print_heat_time(2);
                }
            }
        }
    }
    else if (middle_print_mgr.print_sta == MIDDLE_PRT_STA_WAIT)
    {
        if (middle_print_motor_linefeed_completed())
        {
            /* 马达行进一行完成，开始进行下一行数据进行打印 */
            middle_print_line_data_latch();
        }
        else
        {
            /* 等待走纸完成 */
            hal_print_heat_time(2);
        }
    }

    return 0;
}

// 每行电机步数
int middle_print_motor_linefeed_step(void)
{
    return MOTOR_LINEFEED_STEP;
}

// 判断是否正在打印
int middle_print_is_busy(void)
{
    return (MIDDLE_PRT_STA_IDLE != middle_print_mgr.print_sta);
}

// 打印开始
static int middle_print_start(void)
{
    HAL_PRT_MOTOR_ENABLE();
    middle_print_mgr.print_sta = MIDDLE_PRT_STA_IDLE;
    middle_print_mgr.slice_number = 0;
    middle_print_mgr.slice_total = middle_print_list->segment_number;
    middle_print_mgr.slice_heat_time = middle_print_list->heat_time;

    middle_print_mgr.motor_sta = MIDDLE_PRT_MTR_STA_FEED;
    middle_print_mgr.motor_step = 0;
    middle_print_mgr.motor_step_total = 0;
    middle_print_mgr.motor_step_move = 0;
    middle_print_mgr.motor_speed_time_last = middle_print_list->motor_speed_time;
    middle_print_mgr.motor_speed_time = middle_print_list->motor_speed_time;
    middle_print_mgr.motor_step_target = 0xffffffff;
    hal_print_motor_speed_time(middle_print_mgr.motor_speed_time);
    HAL_LOGD("MID PRN", "print start\r\n");
    return 0;
}

// 打印结束
static int middle_print_stop(void)
{
    hal_print_stop();
    HAL_PRT_MOTOR_DISABLE();
    middle_print_mgr.print_sta = MIDDLE_PRT_STA_IDLE;
    middle_print_mgr.motor_sta = MIDDLE_PRT_MTR_STA_IDLE;
    HAL_LOGD("MID PRN", "print stop\r\n");
    return 0;
}

// 打印处理，循环调用
int middle_print_progress(void)
{
    if (middle_print_list_is_empty())
    {
        return 0;
    }

    if (middle_print_mgr.print_sta == MIDDLE_PRT_STA_IDLE)
    {
        /* 取出数据 */
        middle_print_list = middle_print_list_get();
        if (middle_print_list == NULL)
        {
            return -1;
        }

        /* 启动打印工作 */
        middle_print_start();
        middle_print_line_data_latch();
    }

    return 0;
}

// 打印初始化
int middle_print_init(void)
{
    memset(&middle_print_mgr, 0, sizeof(middle_print_mgr));
    middle_print_mgr.print_sta = MIDDLE_PRT_STA_IDLE;
    middle_print_mgr.motor_sta = MIDDLE_PRT_MTR_STA_IDLE;
    return 0;
}
