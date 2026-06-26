#include "ir.h"

/**
 * @brief     红外遥控输入捕获回调1（上升沿/下降沿检测）
 * @param     无
 * @retval    无
 */
void IR_Input_Choose1(void)
{
    uint16_t dval; /* 下降沿时捕获计数值 */

    if (RDATA) /* 上升沿捕获 */
    {          /* 一定要先清除原来的设置 */
        TIM_RESET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1);
        TIM_SET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1,
                                TIM_ICPOLARITY_FALLING); /* 设置为下降沿捕获 */
        __HAL_TIM_SET_COUNTER(&g_tim1_handle, 0);        /* 清空定时器值 */
        g_remote_sta |= 0X10;                            /* 标记上升沿已经被捕获 */
    }
    else /* 下降沿捕获 */
    {    /* 读取CCR1也可以清CC1IF标志位 */
        dval = HAL_TIM_ReadCapturedValue(&g_tim1_handle, TIM_CHANNEL_1);
        /* 一定要先清除原来的设置 */
        TIM_RESET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1);
        TIM_SET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1,
                                TIM_ICPOLARITY_RISING); /* 设置TIM5通道1上升沿捕获 */

        if (g_remote_sta & 0X10) /* 完成一次高电平捕获 */
        {
            if (g_remote_sta & 0X80) /* 接收到引导码 */
            {
                if (dval > 300 && dval < 800) /* 560为标准值,560us */
                {
                    g_remote_data >>= 1;          /* 右移一位. */
                    g_remote_data &= ~0x80000000; /* 接收到0 */
                }
                else if (dval > 1400 && dval < 1800) /* 1680为标准值,1680us */
                {
                    g_remote_data >>= 1;         /* 右移一位*/
                    g_remote_data |= 0x80000000; /* 接收到1 */
                }
                else if (dval > 2000 && dval < 3000)
                {                         /* 得到按键连发信息 2250为标准值2.25ms */
                    g_remote_cnt++;       /* 按键次数增加1次 */
                    g_remote_sta &= 0XF0; /* 清空计时器 */
                }
            }
            else if (dval > 4200 && dval < 4700) /* 4500为标准值4.5ms */
            {
                g_remote_sta |= 1 << 7; /* 标记成功接收到引导码 */
                g_remote_cnt = 0;       /* 清除按键次数计数器 */
            }
        }
        g_remote_sta &= ~(1 << 4);
    }

    void IR_Input_Choose2(void)
    {
        if (g_remote_sta & 0x80) /* 上次数据还未被取走 */
        {
            g_remote_sta &= ~0X10; /* 取消上升沿已经被捕获标记 */

            if ((g_remote_sta & 0X0F) == 0X00)
            {
                g_remote_sta |= 1 << 6; /* 标记已经完成一次按键的键值信息采集 */
            }

            if ((g_remote_sta & 0X0F) < 14)
            {
                g_remote_sta++;
            }
            else
            {
                g_remote_sta &= ~(1 << 7); /* 清空引导标识 */
                g_remote_sta &= 0XF0;      /* 清空计数器 */
            }
        }
    }
}

/**
 * @brief     红外遥控按键(类似按键扫描)
 * @param     无
 * @retval    0   , 没有任何按键按下
 *              其它, 按下的按键值
 */
uint8_t remote_scan(void)
{
    uint8_t sta = 0;
    uint8_t t1, t2;

    if (g_remote_sta & (1 << 6)) /* 得到一个完成的所有信息了 */
    {
        t1 = g_remote_data;               /* 得到地址码 */
        t2 = (g_remote_data >> 8) & 0xff; /* 得到地址反码 */

        if ((t1 == (uint8_t)~t2) && t1 == REMOTE_ID)
        { /* 检验遥控识别码(ID)及地址 */
            t1 = (g_remote_data >> 16) & 0xff;
            t2 = (g_remote_data >> 24) & 0xff;

            if (t1 == (uint8_t)~t2)
            {
                sta = t1; /* 键值正确 */
            }
        }

        if ((sta == 0) || ((g_remote_sta & 0X80) == 0))
        {                              /* 按键数据错误/遥控已经没有按下 */
            g_remote_sta &= ~(1 << 6); /* 清除接收到有效数据标识 */
            g_remote_cnt = 0;          /* 清除按键次数计数器 */
        }
    }

    return sta;
}
