#ifndef _HAL_PRINT_H_
#define _HAL_PRINT_H_

#include "hal_config.h"

/* #TPH-LAT */
#define HAL_PRT_NLAT_CLOCK
#define HAL_PRT_NLAT_PORT           GPIOA
#define HAL_PRT_NLAT_PIN            GPIO_Pin_15
#define HAL_PRT_NLAT_HIGH()         GPIO_SetBits(HAL_PRT_NLAT_PORT, HAL_PRT_NLAT_PIN);
#define HAL_PRT_NLAT_LOW()          GPIO_ResetBits(HAL_PRT_NLAT_PORT, HAL_PRT_NLAT_PIN);

/* TPH-STB */
#define HAL_PRT_STB_CLOCK
#define HAL_PRT_STB_PORT            GPIOB
#define HAL_PRT_STB_PIN             GPIO_Pin_2
#define HAL_PRT_STB_HIGH()          // GPIO_SetBits(HAL_PRT_STB_PORT, HAL_PRT_STB_PIN);
#define HAL_PRT_STB_LOW()           GPIO_ResetBits(HAL_PRT_STB_PORT, HAL_PRT_STB_PIN);

/* TPH-MOTOR-SLP */
#define HAL_PRT_MOTOR_SLEEP_CLOCK
#define HAL_PRT_MOTOR_SLEEP_PORT    GPIOA
#define HAL_PRT_MOTOR_SLEEP_PIN     GPIO_Pin_3
#define HAL_PRT_MOTOR_ENABLE()      GPIO_SetBits(HAL_PRT_MOTOR_SLEEP_PORT, HAL_PRT_MOTOR_SLEEP_PIN);
#define HAL_PRT_MOTOR_DISABLE()     GPIO_ResetBits(HAL_PRT_MOTOR_SLEEP_PORT, HAL_PRT_MOTOR_SLEEP_PIN);
/* TPH-MOTOR-SPWM */
#define HAL_PRT_MOTOR_A1(x)         {TIM_PCNT(TIM0)=x;TIM_NCNT(TIM0)=480-x;}
#define HAL_PRT_MOTOR_A2(x)         {TIM_PCNT(TIM1)=x;TIM_NCNT(TIM1)=480-x;}
#define HAL_PRT_MOTOR_B1(x)         {TIM_PCNT(TIM2)=x;TIM_NCNT(TIM2)=480-x;}
#define HAL_PRT_MOTOR_B2(x)         {TIM_PCNT(TIM3)=x;TIM_NCNT(TIM3)=480-x;}

int hal_print_send_data(uint8_t *data, uint16_t datalen);
int hal_print_heat_time(uint32_t time);
int hal_print_stop(void);
int hal_print_motor_speed_time(uint32_t time);
int hal_print_motor_stop(void);
int hal_print_temprature(void);
int hal_print_init(void);

#endif
