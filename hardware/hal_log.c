/*******************************************************
 * File Name    : hal_log.c
 * Description  : hardware log unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-23   |         oem
********************************************************/
#include "hal_log.h"
#include "hal_config.h"

void hal_log_printf(const char *fmt, ...)
{
    __va_list ap;
    char printbuf[256] = {0};

    va_start(ap, fmt);
    int printbufsize = vsnprintf(printbuf, 256, fmt, ap);
    UART_SendBuf(UART0, (uint8_t *)printbuf, printbufsize);
    va_end(ap);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    UART_SendData(UART0, (uint8_t)ch);
    return ch;
}

int hal_log_init(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports RX and TX for IO. */
    GPIO_Config(HAL_LOG_TX_PORT, HAL_LOG_TX_PIN, UART0_TXD);
    GPIO_Config(HAL_LOG_RX_PORT, HAL_LOG_RX_PIN, UART0_RXD);

    /* USARTx configured as follow:
    - BaudRate = 115200 baud
    - Word Length = 8 Bits
    - Stop Bit = 1 Stop Bit
    - Parity = No Parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit enabled
    */
    UART_InitStruct.BaudRate = HAL_LOG_UARTBAUD;
    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_1;
    UART_InitStruct.Parity = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
    UART_ITConfig(UART0, UART_IT_RX, ENABLE);
    UART_SetRxITNum(UART0, 10);
    UART_SetITTimeout(UART0, 0xff);
    return 0;
}

// void UART0_IRQHandler(void)
// {
//     if (UART_IsRXFIFONotEmpty(UART0))
//     {
//         UART_RecvBuf(UART0, buf, 9);
//     }
// }
