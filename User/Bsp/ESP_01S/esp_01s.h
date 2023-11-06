#ifndef _ESP_01S_H_
#define _ESP_01S_H_

#include "bsp.h"

void ParsingEsp01sMsg_test(uint8_t *data, uint32_t lenth);
void esp_01s_TxData(uint8_t *_DataBuf, uint32_t _Len);
void esp_01s_RxData(void);
void bsp_esp01s_Init(void);
HAL_StatusTypeDef esp_01s_Init(void);

#endif