#ifndef _MIDDLE_CONF_H_
#define _MIDDLE_CONF_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MIDDLE_PRINT_DOTS           (576)  // 300DPI
#define MIDDLE_PRINT_HEAT_DOTS_MAX  (64)   // 最大加热点数64 
#define MIDDLE_PRINT_HEAT_SEG_MAX   (MIDDLE_PRINT_DOTS / MIDDLE_PRINT_HEAT_DOTS_MAX)
#define MIDDLE_PRINT_LIST_MAX       (10)   // 缓存10行打印数据
#define MIDDLE_PRINT_DATA_SIZE      (MIDDLE_PRINT_DOTS / 8)

#endif
