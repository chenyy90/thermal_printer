#ifndef _HAL_PRINT_H_
#define _HAL_PRINT_H_

#include "hal_config.h"

/* #TPH-LAT */
#define HAL_PRT_NLAT_CLOCK
#define HAL_PRT_NLAT_PORT
#define HAL_PRT_NLAT_PIN
#define HAL_PRT_NLAT_HIGH()
#define HAL_PRT_NLAT_LOW()

/* TPH-STB */
#define HAL_PRT_STB_CLOCK
#define HAL_PRT_STB_PORT
#define HAL_PRT_STB_PIN
#define HAL_PRT_STB_HIGH()
#define HAL_PRT_STB_LOW()
#define HAL_PRT_DI_DMA_ENABLE(size)
#define HAL_PRT_DI_DMA_DISABLE()

/* TPH-MOTOR-SLP */
#define HAL_PRT_MOTOR_SLEEP_CLOCK
#define HAL_PRT_MOTOR_SLEEP_PORT
#define HAL_PRT_MOTOR_SLEEP_PIN
#define HAL_PRT_MOTOR_ENABLE()
#define HAL_PRT_MOTOR_DISABLE()
/* TPH-MOTOR-SPWM */
#define HAL_PRT_MOTOR_A1(x)
#define HAL_PRT_MOTOR_A2(x)
#define HAL_PRT_MOTOR_B1(x)
#define HAL_PRT_MOTOR_B2(x)

int hal_print_heat_time(uint32_t time);
int hal_print_stop(void);
int hal_print_motor_speed_time(uint32_t time);
int hal_print_motor_stop(void);
int hal_print_temprature(void);
int hal_print_init(void);

#endif
