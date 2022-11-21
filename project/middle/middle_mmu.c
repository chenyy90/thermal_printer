/*******************************************************
 * File Name    : middle_mmu.c
 * Description  : memery manager unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-02   |         oem
********************************************************/
#include <stdio.h>
#include <string.h>
#include "middle_mmu.h"

// static __align(4) uint8_t middle_mmu_base[MIDDLE_MMU_MAX_SIZE];
static uint8_t middle_mmu_base[MIDDLE_MMU_MAX_SIZE];
static uint16_t middle_mmu_mapbase[MIDDLE_MMU_MAP_SIZE];

void middle_mmu_init(void)
{  
    memset(&middle_mmu_base[0], 0, sizeof(middle_mmu_base));
    memset(&middle_mmu_mapbase[0], 0, sizeof(middle_mmu_mapbase));
}

void *middle_mmu_alloc(uint8_t num, uint32_t size)
{  
    int offset = 0;
    uint16_t need_blks = size / MIDDLE_MMU_BLOCK_SIZE;
    uint16_t free_blks = 0;

    if (size == 0)
    {
        return NULL;
    }

    /* 补全需要分配的连续内存块数 */
    if (size % MIDDLE_MMU_BLOCK_SIZE) need_blks++;

    /* 搜索整个内存控制区，碰到连续块的，就可以分配 */
    for (offset=MIDDLE_MMU_MAP_SIZE-1; offset>=0; offset--)
    {     
        if (middle_mmu_mapbase[offset] == 0)
        {
            free_blks++;
        }
        else
        {
            free_blks = 0;
        }

        if (free_blks == need_blks)
        {
            for (free_blks=0; free_blks<need_blks; free_blks++)
            {  
                middle_mmu_mapbase[offset + free_blks] = need_blks;  
            }

            return (void *)(middle_mmu_base + offset * MIDDLE_MMU_BLOCK_SIZE);
        }
    }

    return NULL;
}  

int middle_mmu_free(void *ptr)
{
    uint32_t offset = (uint32_t)ptr - (uint32_t)middle_mmu_base;

    /* 偏移所在内存块号码，然后从MAP中得到内存块数量，从而进行内存块清零 */
    if (offset >= MIDDLE_MMU_MAX_SIZE)
    {
        printf("mmu free error");
        return -1;
    }

    int index = offset / MIDDLE_MMU_BLOCK_SIZE;
    int nmemb = middle_mmu_mapbase[index];
    for(int i=0; i<nmemb; i++)
    {  
        middle_mmu_mapbase[index+i] = 0;  
    }

    return 0;
}
