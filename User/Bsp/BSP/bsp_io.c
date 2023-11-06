#include "main.h"
#include "bsp_io.h"
#include "tim.h"

// void io_Motor(void)
//{
//     HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);//����PWM
//     __HAL_RCC_GPIOB_CLK_ENABLE();
//     __HAL_RCC_TIM2_CLK_ENABLE();
//     __HAL_RCC_AFIO_CLK_ENABLE();
//    // __HAL_AFIO_REMAP_TIM2_PARTIAL();
// }
//
// void led()
//{
//   LED_Toggle;
// }
//
// void TIM_SetTIM3Compare4(uint32_t compare)
//{
//     TIM2->CCR4=compare;
// }