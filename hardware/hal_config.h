#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "type.h"
#include "yc3121.h"
#include "yc_sysctrl.h"
#include "yc_systick.h"
#include "yc_adc.h"
#include "yc_bt.h"
#include "yc_gpio.h"
#include "yc_exti.h"
#include "yc_spi.h"
#include "yc_power.h"
#include "yc_qspi.h"
#include "misc.h"

#ifndef SystemCoreClock
#define SystemCoreClock         (96000000)            /* 96MHz */
#endif
#define SEC(X)                  (X * SystemCoreClock) /* MAX 44.7s */
#define MSEC(X)                 (SystemCoreClock / 1000 * X)
#define USEC(X)                 (SystemCoreClock / 1000000 * X)

/* 中断优先等级设置 */
#define HAL_IRQ_HEAT_PRIORITY   (0)
#define HAL_IRQ_MOTOR_PRIORITY  (1)
#define HAL_IRQ_SPIDMA_PRIORITY (2)
#define HAL_IRQ_BUTTON_PRIORITY (3)
#define HAL_IRQ_TIMER_PRIORITY  (4)


#endif
