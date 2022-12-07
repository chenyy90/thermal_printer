#ifndef _HAL_TIME_H_
#define _HAL_TIME_H_

#include "hal_config.h"

typedef int (*hal_time_period_exec_func)(void);

int hal_time_period_execute_register(hal_time_period_exec_func exec_func);
int hal_time_init(void);

#endif
