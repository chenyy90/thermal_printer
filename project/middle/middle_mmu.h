#ifndef _MIDDLE_MMU_H_
#define _MIDDLE_MMU_H_

#include <stdint.h>

#define MIDDLE_MMU_MAX_SIZE   (6 * 1024)                                      // 最大管理内存 6K
#define MIDDLE_MMU_BLOCK_SIZE (32)                                            // 内存块大小为32字节
#define MIDDLE_MMU_MAP_SIZE   (MIDDLE_MMU_MAX_SIZE / MIDDLE_MMU_BLOCK_SIZE)   // 内存表大小

void middle_mmu_init(void);
void *middle_mmu_alloc(uint8_t num, uint32_t size);
int middle_mmu_free(void *ptr);

#endif
