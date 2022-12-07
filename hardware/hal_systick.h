#ifndef _HAL_SYSTICK_H_
#define _HAL_SYSTICK_H_

#include "hal_config.h"

uint32_t hal_systick_time(void);
int hal_systick_delay(uint32_t t);

int hal_systick_init(void);

#endif
