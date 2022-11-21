/*
File Name    : yc_touch.h
Author       : Yichip
Version      : V1.0
Date         : 2020/1/14
Description  :
*/
#ifndef __YC_TOUCH_H__
#define __YC_TOUCH_H__

#include "yc_st7789.h"
#include "yc_adc.h"
#define X 0
#define Y 1

//extern struct TOUCH _touch;
//extern struct TOUCH_PANEl _touchpanel;

struct TOUCH
{
    uint16_t Adc_Max;
    uint8_t PressAgainFalg;
};


//存放触摸屏信息的结构体
struct TOUCH_PANEl
{
    uint16_t x0; //逻辑坐标x
    uint16_t y0; //逻辑坐标y
    uint16_t ltx;  //上个点像素坐标x
    uint16_t lty;  //上个点像素坐标y
    uint16_t x;  //像素坐标x
    uint16_t y;  //像素坐标y
    uint16_t xbase; //x基物理坐标
    uint16_t ybase; //y基物理坐标
    uint16_t xlogic; //x基逻辑坐标
    uint16_t ylogic; //y基逻辑坐标
    uint16_t xfac;//X轴方向比例因子
    uint16_t yfac;//Y轴方向比例因子
};


void TP_ReadXY(void);
uint8_t TP_UpdateCoordVal(void);
uint8_t TP_DetectPressing(void);

#endif
