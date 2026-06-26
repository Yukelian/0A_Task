#include "esp_01s.h"

uint8_t esp01s_RxData[64];
uint8_t esp_char;

/*
*********************************************************************************************************
*	函 数 名: bsp_esp01s_Init
*	功能说明: ESP-01S 接口初始化，绑定收发回调
*	形    参: 无
*	返 回 值: 无
*   备    注:
*********************************************************************************************************
*/
void bsp_esp01s_Init(void)
{
    // BSP_UART1 = ESP_01S;
    BSP_UART1.UART1_RxReceive = esp_01s_RxData;
    BSP_UART1.UART1_TxSend = esp_01s_TxData;
    HAL_UART_Receive_DMA(&huart1, &esp_char, 1);
}
/*
*********************************************************************************************************
*	函 数 名: esp01s_Init
*	功能说明: ESP-01S 上电初始化
*	形    参: 无
*	返 回 值: HAL 状态
*   备    注:
*********************************************************************************************************
*/

HAL_StatusTypeDef esp_01s_Init(void)
{

    //    esp_01s_TxData("AT\r\n",strlen("AT\r\n"));
    //    delay_ms(1000);
    //    esp_01s_TxData("AT+CWMODE=2\r\n",strlen("AT+CWMODE=2\r\n"));
    //    delay_ms(1000);
    //    esp_01s_TxData("AT+CWJAP=\"TP-LINK_3E30\",\"18650711783\"\r\n",strlen("AT+CWJAP=\"TP-LINK_3E30\",\"18650711783\"\r\n"));
    //    delay_ms(1000);
    esp_01s_TxData("AT+CIPMUX=1\r\n", strlen("AT+CIPMUX=1\r\n")); // 上电需要重新发送指令，单连接模式
    delay_ms(100);
    esp_01s_TxData("AT+CIPSERVER=1,8080\r\n", strlen("AT+CIPSERVER=1,8080\r\n")); // 开启服务器端口监听
    //    delay_ms(1000);
    //    esp_01s_TxData("AT+CIFSR\r\n",strlen("AT+CIFSR\r\n"));
    //    delay_ms(1000);
    //    if(strcmp(esp01s_RxData, "OK") != 0)
    //      return HAL_ERROR;

    return HAL_OK;
}

/*
*********************************************************************************************************
*	函 数 名: esp_01s_TxData
*	功能说明: ESP-01S 数据发送
*	形    参: _DataBuf 数据缓冲区, _Len 数据长度
*	返 回 值: 无
*   备    注:
*********************************************************************************************************
*/
void esp_01s_TxData(uint8_t *_DataBuf, uint32_t _Len)
{
    HAL_UART_Transmit_DMA(&huart1, _DataBuf, _Len);
}

/*
*********************************************************************************************************
*	函 数 名: esp_01s_RxData
*	功能说明: ESP-01S 数据接收回调
*	形    参: 无
*	返 回 值: 无
*   备    注:
*********************************************************************************************************
*/
void esp_01s_RxData(void)
{
    static uint8_t i = 0;
    if (esp_char != '\n')
    {
        esp01s_RxData[i] = esp_char;
        i++;
    }
    else
    {
        ParsingEsp01sMsg_test(esp01s_RxData, i);
        i = 0;
    }
    HAL_UART_Receive_DMA(&huart1, &esp_char, 1);
}

/*
*********************************************************************************************************
*	函 数 名: ParsingEsp01sMsg_test
*	功能说明: ESP-01S 消息解析测试
*	形    参: data 接收数据, lenth 数据长度
*	返 回 值: 无
*   备    注:
*********************************************************************************************************
*/
#define esp_cmd data[0]
void ParsingEsp01sMsg_test(uint8_t *data, uint32_t lenth)
{
    if (esp_cmd == '1')
    {
        esp_01s_TxData("111", 3);
    }
    else if (esp_cmd == '2')
    {
        esp_01s_TxData("222", 3);
    }
    else if (esp_cmd == '3')
    {
        esp_01s_TxData("333", 3);
    }
}
