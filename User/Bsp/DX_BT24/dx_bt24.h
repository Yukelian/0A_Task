#ifndef _DX_BT24_H_
#define _DX_BT24_H_

#include "bsp.h"

void bsp_dx_bt24_Init(void);
void ParsingDxbt24Msg_test(uint8_t *data, uint32_t lenth);
void dx_bt24_RxData(void);
void dx_bt24_TxData(uint8_t *_DataBuf, uint32_t _Len);
HAL_StatusTypeDef dx_bt24_Init(void);

#endif