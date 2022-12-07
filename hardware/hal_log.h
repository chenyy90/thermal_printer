#ifndef _HAL_LOG_H_
#define _HAL_LOG_H_

#include "hal_systick.h"

#define HAL_LOG_UARTBAUD      921600

#define HAL_LOG_TX_PORT       GPIOA
#define HAL_LOG_TX_PIN        GPIO_Pin_1
#define HAL_LOG_RX_PORT       GPIOA
#define HAL_LOG_RX_PIN        GPIO_Pin_0

void hal_log_printf(const char *fmt, ...);

#ifndef hal_systick_time
#define hal_systick_time() "  "
#endif

#define HAL_LOGD(tag, fmt, args...) hal_log_printf("[%lu] |%-8.8s| "fmt, hal_systick_time(), tag, ##args)
#define HAL_LOGW(tag, fmt, args...) hal_log_printf("[%lu] |%-8.8s| "fmt, hal_systick_time(), tag, ##args)
#define HAL_LOGE(tag, fmt, args...) hal_log_printf("[%lu] |%-8.8s| "fmt, hal_systick_time(), tag, ##args)

int hal_log_init(void);

#endif
