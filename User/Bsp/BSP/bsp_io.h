#ifndef _BSP_IO_H_
#define _BSP_IO_H_

#include "stdint.h"
//#include "stm32f1xx_hal_gpio.h"

#define LED(n) (n?HAL_GPIO_WritePin(Out_Led_GPIO_Port, Out_Led_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(Out_Led_GPIO_Port, Out_Led_Pin,GPIO_PIN_RESET))
#define LED_Toggle HAL_GPIO_TogglePin(Out_Led_GPIO_Port, Out_Led_Pin) //LED0 �����ƽ��ת
#define KEY_1 HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)

void led();
void TIM_SetTIM3Compare4(uint32_t compare);
#endif
