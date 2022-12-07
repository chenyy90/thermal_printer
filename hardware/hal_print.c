/*******************************************************
 * File Name    : hal_print.c
 * Description  : hardware print init unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-23   |         oem
********************************************************/
#include "hal_print.h"
#include "middle_print.h"

int hal_print_send_data(uint8_t *data, uint16_t datalen)
{
    DMA_SRC_ADDR(SPI0) = (int)data;
    DMA_LEN(SPI0) = (datalen << 16);
    DMA_START(SPI0) = (1 << DMA_START_BIT);
    while (!(DMA_STATUS(SPI0) & 1));
    middle_print_latch_to_heat_handle();
    return 0;
}

int hal_print_heat_time(uint32_t time)
{
    TIM_Cmd(TIM5, DISABLE);
    TIM_SetPeriod(TIM5, USEC(time));
    TIM_Cmd(TIM5, ENABLE);
    return 0;
}

int hal_print_stop(void)
{
    HAL_PRT_STB_LOW();
    TIM_Cmd(TIM5, ENABLE);
    return 0;
}

int hal_print_motor_speed_time(uint32_t time)
{
    TIM_Cmd(TIM4, DISABLE);
    TIM_SetPeriod(TIM4, USEC(time));
    TIM_Cmd(TIM4, ENABLE);
    return 0;
}

int hal_print_motor_stop(void)
{
    HAL_PRT_MOTOR_DISABLE();
    TIM_Cmd(TIM4, ENABLE);

    HAL_PRT_MOTOR_A1(0);
    HAL_PRT_MOTOR_A2(0);
    HAL_PRT_MOTOR_B1(0);
    HAL_PRT_MOTOR_B2(0);
    return 0;
}

int hal_print_temprature(void)
{
    return 0;
}

static void hal_print_motor_spwm_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    PWM_InitTypeDef PWM_InitStruct;

    /* MOTOR-SLEEP */
    GPIO_InitStruct.GPIO_Pin = HAL_PRT_MOTOR_SLEEP_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HAL_PRT_MOTOR_SLEEP_PORT, &GPIO_InitStruct);
    HAL_PRT_MOTOR_DISABLE();

    /* PWM Init */
    PWM_InitStruct.LowLevelPeriod = 480;
    PWM_InitStruct.HighLevelPeriod = 0;
    PWM_InitStruct.SatrtLevel = OutputLow;

    // A1: PA5, A2: PA4, B1: PA11, B2: PA10
    GPIO_Config(GPIOA, GPIO_Pin_5, PWM_OUT0);
    GPIO_Config(GPIOA, GPIO_Pin_4, PWM_OUT1);
    GPIO_Config(GPIOA, GPIO_Pin_11, PWM_OUT2);
    GPIO_Config(GPIOA, GPIO_Pin_10, PWM_OUT3);

    PWM_InitStruct.TIMx = TIM0;
    TIM_PWMInit(&PWM_InitStruct);
    TIM_ModeConfig(TIM0, TIM_Mode_PWM);
    TIM_Cmd(TIM0, ENABLE);

    PWM_InitStruct.TIMx = TIM1;
    TIM_PWMInit(&PWM_InitStruct);
    TIM_ModeConfig(TIM1, TIM_Mode_PWM);
    TIM_Cmd(TIM1, ENABLE);

    PWM_InitStruct.TIMx = TIM2;
    TIM_PWMInit(&PWM_InitStruct);
    TIM_ModeConfig(TIM2, TIM_Mode_PWM);
    TIM_Cmd(TIM2, ENABLE);

    PWM_InitStruct.TIMx = TIM3;
    TIM_PWMInit(&PWM_InitStruct);
    TIM_ModeConfig(TIM3, TIM_Mode_PWM);
    TIM_Cmd(TIM3, ENABLE);
}

static void hal_print_motor_speed_init(void)
{
    TIM_InitTypeDef TIM_InitStruct;

    /* Motor Timer */
    TIM_InitStruct.period = USEC(1);
    TIM_InitStruct.TIMx = TIM4;
    TIM_Init(&TIM_InitStruct);
    TIM_ModeConfig(TIM4, TIM_Mode_TIMER);
    TIM_Cmd(TIM4, DISABLE);
    NVIC_EnableIRQ(TIM4_IRQn);
    NVIC_SetPriority(TIM4_IRQn, HAL_IRQ_MOTOR_PRIORITY);
}

static void hal_print_tp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_InitTypeDef TIM_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    /* NLAT */
    GPIO_InitStruct.GPIO_Pin = HAL_PRT_NLAT_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HAL_PRT_NLAT_PORT, &GPIO_InitStruct);
    HAL_PRT_NLAT_HIGH();

    /* STB */
    GPIO_InitStruct.GPIO_Pin = HAL_PRT_STB_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HAL_PRT_STB_PORT, &GPIO_InitStruct);
    HAL_PRT_STB_LOW();

    /* Heat Ctrl Timer */
    TIM_InitStruct.period = 480;
    TIM_InitStruct.TIMx = TIM5;
    TIM_Init(&TIM_InitStruct);
    TIM_ModeConfig(TIM5, TIM_Mode_TIMER);
    TIM_Cmd(TIM5, DISABLE);
    NVIC_EnableIRQ(TIM5_IRQn);
    NVIC_SetPriority(TIM5_IRQn, HAL_IRQ_HEAT_PRIORITY);

    /* TPH-SPI */
    GPIO_Config(GPIOB, GPIO_Pin_1, SPID0_MOSI);
    GPIO_Config(GPIOB, GPIO_Pin_0, SPID0_SCK);

    SPI_InitStruct.Mode = SPI_Mode_Master;
    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.RW_Delay = 1;
    SPI_Init(SPI0, &SPI_InitStruct);
    // NVIC_EnableIRQ(SPI0_IRQn);
    // NVIC_SetPriority(SPI0_IRQn, HAL_IRQ_SPIDMA_PRIORITY);
}

int hal_print_init(void)
{
    hal_print_motor_spwm_init();
    hal_print_motor_speed_init();
    hal_print_tp_init();
    return 0;
}

/* Motor Timer IRQ */
void TIMER4_IRQHandler(void)
{
    middle_print_motor_spwm_handle();
}

/* Heat Ctrl Timer IRQ */
void TIMER5_IRQHandler(void)
{
    middle_print_heat_haddle();
}

// /* TPH SPI IRQ */
// void SPI0_IRQHandler(void)
// {
//     printf("spi irq\r\n");
//     middle_print_latch_to_heat_handle();
// }
