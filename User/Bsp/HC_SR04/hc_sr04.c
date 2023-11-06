#include "hc_sr04.h"
#include "bsp.h"

uint8_t TIM3CH1_CAP_STA;  
uint16_t TIM3CH1_CAP_VAL; 
/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: 
*	形    参: 
*	返 回 值: 
*   备    注：
*********************************************************************************************************
*/
void time_Input_Choose1(void)
{
    if ((TIM3CH1_CAP_STA & 0X80) == 0) 
    {
        if (TIM3CH1_CAP_STA & 0X40) 
        {
            if ((TIM3CH1_CAP_STA & 0X3F) == 0X3F) 
            {
                TIM3CH1_CAP_STA |= 0X80;                       
                TIM3CH1_CAP_VAL = 0XFFFF; 
            }

            else
                TIM3CH1_CAP_STA++;
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: 
*	形    参: 
*	返 回 值: 
*   备    注：
*********************************************************************************************************
*/
void time_Input_Choose2(void)
{
    if ((TIM3CH1_CAP_STA & 0X80) == 0) 
    {
        if (TIM3CH1_CAP_STA & 0X40) 
        {
            TIM3CH1_CAP_STA |= 0X80;                                               
            TIM3CH1_CAP_VAL = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);    
            TIM_RESET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1);                      
            TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING); 
        }
        else
        {
            TIM3CH1_CAP_STA = 0;                                                    
            TIM3CH1_CAP_VAL = 0;                                                    
            TIM3CH1_CAP_STA |= 0X40;                                                
            __HAL_TIM_DISABLE(&htim2);                                              
            __HAL_TIM_SET_COUNTER(&htim2, 0);                                      
            TIM_RESET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1);                       
            TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING); 
            __HAL_TIM_ENABLE(&htim2);                                               
        }
    }
}