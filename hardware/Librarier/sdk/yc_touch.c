/*
File Name    : yc_touch.c
Author       : Yichip
Version      : V1.0
Date         : 2020/1/14
Description  :
*/
#include "yc_touch.h"

struct TOUCH _touch;
struct TOUCH_PANEl _touchpanel;

//058电签板
//uint16_t TP_YHigh_Pin  = GPIO_Pin_10;
//uint16_t TP_YLow_Pin   = GPIO_Pin_11;
//uint16_t TP_XHigh_Pin  = GPIO_Pin_11;
//uint16_t TP_XLow_Pin   = GPIO_Pin_10;

//GPIO_TypeDef TP_YHigh_Port  = GPIOB;
//GPIO_TypeDef TP_YLow_Port   = GPIOC;
//GPIO_TypeDef TP_XHigh_Port  = GPIOB;
//GPIO_TypeDef TP_XLow_Port   = GPIOC;

//#define PRESS_DETECT_CHANNEL  ADC_CHANNEL_5 //需要选择配置上拉模式的GPIO对应通道，用于检测是否触摸
//#define Y_DETECT_CHANNEL  	  ADC_CHANNEL_5 //Y轴AD检测通道
//#define X_DETECT_CHANNEL  	  ADC_CHANNEL_6	//X轴AD检测通道

//109电签板
uint16_t TP_YHigh_Pin  = GPIO_Pin_4;
uint16_t TP_YLow_Pin   = GPIO_Pin_12;
uint16_t TP_XHigh_Pin  = GPIO_Pin_13;
uint16_t TP_XLow_Pin   = GPIO_Pin_11;

GPIO_TypeDef TP_YHigh_Port  = GPIOA;
GPIO_TypeDef TP_YLow_Port   = GPIOC;
GPIO_TypeDef TP_XHigh_Port  = GPIOA;
GPIO_TypeDef TP_XLow_Port   = GPIOC;

#define PRESS_DETECT_CHANNEL  ADC_CHANNEL_6 //需要选择配置上拉模式的GPIO对应通道，用于检测是否触摸
#define Y_DETECT_CHANNEL  	  ADC_CHANNEL_6 //Y轴AD检测通道
#define X_DETECT_CHANNEL  	  ADC_CHANNEL_7	//X轴AD检测通道


/*
函数功能:  得到逻辑坐标
参数：		Y：纵轴
			X：横轴
返回值:  返回的逻辑坐标
*/
#define SAMP_CNT      8  //采样次数
#define SAMP_CNT_DIV2 4   //采样数据中间值
#define THRESHOLD 1  //滤波阈值

#define Yad_min 0x480
#define Xad_min 0x4a0

uint16_t TP_GetCoordVal(uint8_t x_or_y)
{
    uint16_t CoordValAdc[SAMP_CNT] = {0}, CoordVal[SAMP_CNT] = {0}, tempxy = 0;
    uint8_t i = 0, min = 0, j = 0;
    if (x_or_y == X)
    {
        // 配置采样Y轴IO
        GPIO_Config(TP_YLow_Port, TP_YLow_Pin, OUTPUT_LOW);
        GPIO_Config(TP_XHigh_Port, TP_XHigh_Pin, ANALOG);
        GPIO_Config(TP_XLow_Port, TP_XLow_Pin, ANALOG);
        GPIO_Config(TP_YHigh_Port, TP_YHigh_Pin, OUTPUT_HIGH);

        // 采样SAMP_CNT组数据，并计算出逻辑坐标
        for (i = 0; i < SAMP_CNT; i++)
        {
            CoordValAdc[i] = ADC_GetResult(Y_DETECT_CHANNEL);
            if (CoordValAdc[i] < Yad_min)CoordValAdc[i] = Yad_min;
            CoordVal[i] = 320 - ((CoordValAdc[i] - Yad_min) * 0.36);
            if (CoordVal[i] > 320) CoordVal[i] = 0;
            delay_us(10);
        }
    }
    else if (x_or_y == Y)
    {
        //配采样X轴IO
        GPIO_Config(TP_YLow_Port, TP_YLow_Pin, ANALOG);
        GPIO_Config(TP_XHigh_Port, TP_XHigh_Pin, OUTPUT_HIGH);
        GPIO_Config(TP_XLow_Port, TP_XLow_Pin, OUTPUT_LOW);
        GPIO_Config(TP_YHigh_Port, TP_YHigh_Pin, ANALOG);

        //采样SAMP_CNT组数据，并计算出逻辑坐标
        for (i = 0; i < SAMP_CNT; i++)
        {
            CoordValAdc[i] = ADC_GetResult(X_DETECT_CHANNEL);
            if (CoordValAdc[i] < Xad_min)CoordValAdc[i] = Xad_min;
            CoordVal[i] = 240 - (CoordValAdc[i] - Xad_min) * 0.28;
            if (CoordVal[i] > 240) CoordVal[i] = 0;
            delay_us(10);
        }
    }

    //滤波
    for (i = 0; i < SAMP_CNT - 1; i++)
    {
        min = i;
        for (j = i + 1; j < SAMP_CNT; j++)
        {
            if (CoordVal[min] > CoordVal[j]) min = j;
        }
        tempxy = CoordVal[i];
        CoordVal[i] = CoordVal[min];
        CoordVal[min] = tempxy;
    }

    if ((CoordVal[SAMP_CNT_DIV2] - CoordVal[SAMP_CNT_DIV2 - 1]) > THRESHOLD) return 0;
    tempxy = (CoordVal[SAMP_CNT_DIV2] + CoordVal[SAMP_CNT_DIV2 - 1]) / 2;
    return tempxy;
}

/*
函数功能: 检测触摸屏是否按下
返回值:  0:按下
		 1:未按下
*/
uint8_t TP_DetectPressing(void)
{
    uint16_t AdcPress = 0;
    GPIO_Config(TP_YLow_Port, TP_YLow_Pin, OUTPUT_LOW);
    GPIO_Config(TP_XHigh_Port, TP_XHigh_Pin, ANALOG);
    GPIO_Config(TP_XLow_Port, TP_XLow_Pin, PULL_UP);
    GPIO_Config(TP_YHigh_Port, TP_YHigh_Pin, OUTPUT_HIGH);
    AdcPress = ADC_GetResult(PRESS_DETECT_CHANNEL);
    if ((AdcPress + 3)  < _touch.Adc_Max)return 0;
    else return 1;
}
/*
函数功能:  由逻辑坐标得到物理坐标
*/
void TP_ReadXY(void)
{
    uint16_t diff_x = 0, diff_y = 0;

    /*1. 得到逻辑坐标*/

    _touchpanel.x0 = TP_GetCoordVal(X);
    _touchpanel.y0 = TP_GetCoordVal(Y);



    /*2. 得到物理坐标*/

    if (_touchpanel.x0 > _touchpanel.xlogic)
        _touchpanel.x = _touchpanel.xbase + ((_touchpanel.x0 - _touchpanel.xlogic) * 100) / _touchpanel.xfac;
    else
    {
        diff_x = _touchpanel.xlogic - _touchpanel.x0;
        if (diff_x < _touchpanel.xbase)
            _touchpanel.x = _touchpanel.xbase - (diff_x * 100) / _touchpanel.xfac;
        else
            _touchpanel.x = 1;
    }


    if (_touchpanel.y0 > _touchpanel.ylogic)
        _touchpanel.y = _touchpanel.ybase + ((_touchpanel.y0 - _touchpanel.ylogic) * 100) / _touchpanel.yfac;
    else
    {
        diff_y = _touchpanel.ylogic - _touchpanel.y0 ;
        if (diff_y < _touchpanel.ybase)
            _touchpanel.y = _touchpanel.ybase - (diff_y * 100) / _touchpanel.yfac;
        else
        {
            _touchpanel.y = 1;
        }

    }
}

/*
函数功能:  触摸屏校准、更新坐标
返回值:  0 ：校准失败
		 1 ：校准成功
*/
#define UPDTATE_DIFFERENCE 20
uint8_t TP_UpdateCoordVal(void)
{
    uint8_t cnt = 0, difference_x1 = 0, difference_x2 = 0, difference_y1 = 0, difference_y2 = 0;
    uint32_t time_cnt = 0;
    uint16_t x, y;
    uint16_t x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;

    /*1. 判断是否校准过*/

    //画第一个圆
    St7789_tft_DrawCircle(30, 30, 20, RED);
    while (1)
    {
        if (TP_DetectPressing() == 0) //表示触摸屏按下
        {
            x = TP_GetCoordVal(X);
            y = TP_GetCoordVal(Y);
            while (TP_DetectPressing() == 0) {} //等待松开
            cnt++; //记录按下的次数
        }
        switch (cnt)
        {
        case 1:
            x1 = x;
            y1 = y;
            //清除第一个圆
            St7789_tft_DrawCircle(30, 30, 20, WHITE);
            //画第二个圆
            St7789_tft_DrawCircle(30, 240 - 30, 20, RED);
            break;
        case 2:
            x2 = x;
            y2 = y;
            //清除第二个圆
            St7789_tft_DrawCircle(30, 240 - 30, 20, WHITE);
            //画第三个圆
            St7789_tft_DrawCircle(320 - 30, 30, 20, RED);
            break;
        case 3:
            x3 = x;
            y3 = y;
            //清除第三个圆
            St7789_tft_DrawCircle(320 - 30, 30, 20, WHITE);
            //画第四个圆
            St7789_tft_DrawCircle(320 - 30, 240 - 30, 20, RED);
            break;
        case 4:
            x4 = x;
            y4 = y;
            //清除第四个圆
            St7789_tft_DrawCircle(320 - 30, 240 - 30, 20, WHITE);
            //画第五个个圆
            St7789_tft_DrawCircle(160, 120, 20, RED);
            break;
        case 5:
            x5 = x;
            y5 = y;
            //清除第五个圆
            St7789_tft_DrawCircle(160, 120, 20, WHITE);

            //检测坐标差值
            if (x2 > x1) difference_x1 = x2 - x1;
            else difference_x1 = x1 - x2;
            if (x3 > x4) difference_x2 = x3 - x4;
            else difference_x2 = x4 - x3;

            if (y3 > y1) difference_y1 = y3 - y1;
            else difference_y1 = y1 - y3;
            if (y2 > y4) difference_y2 = y2 - y4;
            else difference_y2 = y4 - y2;

            if ((difference_x1 < UPDTATE_DIFFERENCE) && (difference_x2 < UPDTATE_DIFFERENCE) \
                    && (difference_y1 < UPDTATE_DIFFERENCE) && (difference_y2 < UPDTATE_DIFFERENCE))
            {
                //计算KX与Ky
                _touchpanel.xfac = (((x3 - x1) * 100) / (320 - 30 - 30) + ((x4 - x2) * 100) / (320 - 30 - 30)) / 2;
                _touchpanel.yfac = (((y2 - y1) * 100) / (240 - 30 - 30) + ((y4 - y3) * 100) / (240 - 30 - 30)) / 2;


                // 以第一个圆的逻辑坐标 用Kx与ky计算出E点的逻辑坐标
                x = x1 + (_touchpanel.xfac * (160 - 30)) / 100;
                y = y1 + (_touchpanel.yfac * (120 - 30)) / 100;


                // 比较触摸得到E点逻辑坐标与计算出E点逻辑坐标的差
                if (x > x5) difference_x1 = x - x5;
                else difference_x1 = x5 - x;
                if (y > y5) difference_y1 = y - y5;
                else difference_y1 = y5 - y;
                if (difference_x1 < UPDTATE_DIFFERENCE && difference_y1 < UPDTATE_DIFFERENCE)
                {
                    //保存基坐标
                    _touchpanel.xbase = 30;
                    _touchpanel.ybase = 30;
                    _touchpanel.xlogic = x1;
                    _touchpanel.ylogic = y1;

                    MyPrintf(" _touchpanel.xfac : %d \n",  _touchpanel.xfac);
                    MyPrintf(" _touchpanel.yfac : %d \n",  _touchpanel.yfac);
                    MyPrintf(" _touchpanel.ybase : %d \n",  _touchpanel.ybase);
                    MyPrintf(" _touchpanel.ybase : %d \n",  _touchpanel.ybase);
                    MyPrintf(" _touchpanel.xlogic : %d \n", _touchpanel.xlogic);
                    MyPrintf(" _touchpanel.ylogic : %d \n", _touchpanel.ylogic);

                    return 1;
                }
                else
                {
                    cnt = 0;
                }
            }
            else
            {
                cnt = 0;
            }
            time_cnt++;
            if (time_cnt >= 3)
            {
#if 1
                _touchpanel.xfac = 102;
                _touchpanel.yfac = 84;
                _touchpanel.xbase = 30;
                _touchpanel.ybase = 30;
                _touchpanel.xlogic = 29;
                _touchpanel.ylogic = 83;
#else
                _touchpanel.xfac = 96;
                _touchpanel.yfac = 114;
                _touchpanel.xbase = 30;
                _touchpanel.ybase = 30;
                _touchpanel.xlogic = 51;
                _touchpanel.ylogic = 26;
#endif

                return 0;

            }
            St7789_tft_DrawCircle(30, 30, 20, RED);
            break;

        }

    }
    return 0;
}
