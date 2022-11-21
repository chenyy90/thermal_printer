#ifndef _MIDDLE_PRINT_H_
#define _MIDDLE_PRINT_H_

#include "middle_conf.h"

int middle_print_is_busy(void);
int middle_print_start(void);
int middle_print_stop(void);
int middle_print_line_data_put(uint8_t *data, uint16_t datalen);

int middle_print_handle(void);
int middle_print_init(void);

#endif
