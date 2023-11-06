#ifndef _BSP_H_
#define _BSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task_start.h"
#include "perf_counter.h"
#include "perf_counter_configure.h"
#include "oled.h"
#include "bsp_io.h"
#include "tim.h"
#include "hc_sr04.h"
#include "usart.h"
#include "esp_01s.h"
#include "spi.h"
#include "w25qxx.h"
#include "gpio.h"
#include "nrf24l01.h"
#include "dx_bt24.h"
void bsp_Init(void);

typedef struct
{
    void (*Input_Choose1)(void); 
    void (*Input_Choose2)(void); 
} Input_time;

typedef struct
{
    uint8_t RxStatus;
    void (*SPI1_TxSend)(uint8_t *, uint8_t);
    uint8_t (*SPI1_RxReceive)(uint8_t *);
} SPI1_TypeDef;

typedef struct
{
    uint8_t RxStatus;
    void (*UART1_Init)(void);
    void (*UART1_TxSend)(uint8_t *, uint32_t);
    void (*UART1_RxReceive)(void);
} UART1_TypeDef;

typedef struct
{
    uint8_t M_ID;
    SPI1_TypeDef _SPI1;
    UART1_TypeDef _UART1;
    UART1_TypeDef _UART2;
} BSP_TypeDef;

extern BSP_TypeDef _BSP;
extern UART1_TypeDef BSP_UART1;

void Input_dev_choose(void);

// extern Input_time Input_dev;
#endif
