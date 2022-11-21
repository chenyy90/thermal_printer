/*******************************************************
 * File Name    : middle_print_list.c
 * Description  : printer print data list.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-02   |         oem
********************************************************/
#include "middle_print_list.h"
#include "middle_mmu.h"

typedef struct
{
    uint16_t list_put_index;
    uint16_t list_get_index;
    middle_print_list_struct list[MIDDLE_PRINT_LIST_MAX];
} middle_printlist_manager_struct;

const uint8_t print_dots_calculate_table[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

static middle_printlist_manager_struct mid_printlistm;

// 打印行数据分割
static int middle_print_line_data_slice(uint8_t *src, uint8_t *dst, uint16_t dst_size, uint16_t *bit_offset, uint16_t ave_bit)
{
    uint16_t offset_start = *bit_offset / 8, offset_bits = *bit_offset % 8;
    uint16_t heatbits = 0;

    // printf("-----------> start %d bits\r\n", offset_start * 8 + offset_bits);
    if (offset_bits)
    {
        dst[offset_start] = src[offset_start] & (0xFF >> offset_bits);
        heatbits = print_dots_calculate_table[dst[offset_start]];
        offset_start++;
    }
    // printf("-----------> heatbits 1 %d\r\n", heatbits);

    while (offset_start < dst_size)
    {
        dst[offset_start] = src[offset_start];
        heatbits += print_dots_calculate_table[dst[offset_start]];
        if (heatbits >= ave_bit)
        {
            break;
        }
        offset_start++;
    }
    // printf("-----------> heatbits 2 %d\r\n", heatbits);

    offset_bits = 0;
    if (heatbits > ave_bit)
    {
        heatbits -= ave_bit;
        while(heatbits && (offset_bits < 8))
        {
            if (dst[offset_start] & (0x01 << offset_bits))
            {
                heatbits--;
            }
            offset_bits++;
        }
        dst[offset_start] &= 0xFF << offset_bits;
        offset_start++;
    }
    else if (heatbits == ave_bit)
    {
        offset_start++;
    }

    *bit_offset = offset_start * 8 - offset_bits;
    // printf("-----------> stop  %d bits\r\n", *bit_offset);
    return 0;
}

// 加热时间计算
static uint32_t middle_print_line_heattime(uint16_t heat_bast_time, uint16_t seg_num, uint16_t seg_ave_dots, int32_t tp_tm)
{
    int32_t heat_time = heat_bast_time;
    int32_t heat_offset = 100;
    #define TEMPERATRUE_ADJUST 8 / 5

    // 加热温度补偿
    heat_time = (heat_time * (100 + 25 * TEMPERATRUE_ADJUST - tp_tm * TEMPERATRUE_ADJUST)) / 100;

    // 加热电压补偿
    heat_time *= 1;

    // 加热点数补偿
    heat_time = heat_time * 8 / 10 + heat_time * 4 * seg_ave_dots / 10 / MIDDLE_PRINT_HEAT_DOTS_MAX;

    // 分段补偿
    heat_offset = heat_offset + (seg_num - 1) * 4;
    heat_time = heat_time * heat_offset / 100;
    return heat_time;
}

// 打印行数据解析
static int middle_print_line_data_parse(middle_print_list_struct *list_data, uint8_t *src_data)
{
    uint16_t i = 0;
    uint32_t heat_dots_total = 0;

    // 计算整行总体加热点数
    for (i=0; i<MIDDLE_PRINT_DATA_SIZE; i++)
    {
        heat_dots_total += print_dots_calculate_table[src_data[i]];
    }

    // 计算加热分段数
    uint16_t seg_num = (heat_dots_total + MIDDLE_PRINT_HEAT_DOTS_MAX - 1) / MIDDLE_PRINT_HEAT_DOTS_MAX;
    if (seg_num == 0) {seg_num = 1;}

    // 计算平均加热点数
    uint16_t seg_ave = (heat_dots_total + seg_num - 1) / seg_num;;

    // 分出数据组
    list_data->segment_number = 0;
    uint16_t seg_bit_offset = 0;
    for (i=0; i<seg_num; i++)
    {
        list_data->segment_data[i] = middle_mmu_alloc(1, MIDDLE_PRINT_DATA_SIZE);
        if (list_data->segment_data[i] == NULL)
        {
            printf("print segment data malloc falied\r\n");
            return -1;
        }

        middle_print_line_data_slice(src_data, list_data->segment_data[i], MIDDLE_PRINT_DATA_SIZE, &seg_bit_offset, seg_ave);
        list_data->segment_number++;
    }
    list_data->segment_ave_dot = seg_ave;

    // 计算该行加热时间
    list_data->heat_time = middle_print_line_heattime(5000, seg_num, seg_ave, 30);

    // 计算电机运行时间
    list_data->motor_speed_time = 2000 + list_data->heat_time / 12;

    // printf("listput : heattime %d, motor %d, segment %d, ave %d\r\n",
    //     list_data->heat_time, list_data->motor_speed_time,
    //     list_data->segment_number, list_data->segment_ave_dot);
    // for (int i=0; i<list_data->segment_number; i++)
    // {
    //     for (int byte=0; byte<MIDDLE_PRINT_DATA_SIZE; byte++)
    //     {
    //         printf("%02x, ", list_data->segment_data[i][byte]);
    //     }
    //     printf("\r\n\r\n");
    // }
    return 0;
}


// 判断打印列表是否空闲
int middle_print_list_is_free(void)
{
    int free_zone = 0;

    if (mid_printlistm.list_put_index >= mid_printlistm.list_get_index)
    {
        free_zone = mid_printlistm.list_put_index - mid_printlistm.list_get_index;
        free_zone = MIDDLE_PRINT_LIST_MAX - free_zone;
    }
    else
    {
        free_zone = mid_printlistm.list_get_index - mid_printlistm.list_put_index;
    }

    return free_zone;
}

// 判断打印列表是否为空
int middle_print_list_is_empty(void)
{
    return (mid_printlistm.list_get_index == mid_printlistm.list_put_index);
}

// 输入打印行数据到打印列表
int middle_print_list_put(uint8_t *line_data, uint16_t line_datalen)
{
    if (line_datalen % MIDDLE_PRINT_DATA_SIZE)
    {
        printf("print line data size error\r\n");
        return -1;
    }

    uint16_t line_num_total = line_datalen / MIDDLE_PRINT_DATA_SIZE;
    if (line_num_total > middle_print_list_is_free())
    {
        printf("print line data size error\r\n");
        return -1;
    }

    uint16_t line_num = 0;
    middle_print_list_struct list;

    for (line_num=0; line_num<line_num_total; line_num++)
    {
        if (middle_print_line_data_parse(&list, &line_data[line_num * MIDDLE_PRINT_DATA_SIZE]))
        {
            for (int i=0; i<list.segment_number; i++)
            {
                middle_mmu_free(list.segment_data[i]);
            }
            return -1;
        }

        memcpy(&mid_printlistm.list[mid_printlistm.list_put_index], &list, sizeof(middle_print_list_struct));
        if (++mid_printlistm.list_put_index >= MIDDLE_PRINT_LIST_MAX)
        {
            mid_printlistm.list_put_index = 0;
        }       
    }

    return 0;
}

// 获取打印列表数据
middle_print_list_struct *middle_print_list_get(void)
{
    if (middle_print_list_is_empty())
    {
        return NULL;
    }

    middle_print_list_struct *list = &mid_printlistm.list[mid_printlistm.list_get_index];
    if (++mid_printlistm.list_get_index >= MIDDLE_PRINT_LIST_MAX)
    {
        mid_printlistm.list_get_index  = 0;
    }
    return list;
}

// 打印列表释放
int middle_print_list_destory(middle_print_list_struct *p)
{
    if (p)
    {
        for (int i=0; i<p->segment_number; i++)
        {
            middle_mmu_free(p->segment_data[i]);
        }
        return 0;
    }
    return -1;
}

// 打印列表初始化
int middle_print_list_init(void)
{
    memset(&mid_printlistm, 0, sizeof(mid_printlistm));
    return 0;
}
