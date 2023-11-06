#include "esp_01s.h"

uint8_t esp01s_RxData[64];
uint8_t esp_char;

/*
*********************************************************************************************************
*	�� �� ��: bsp_esp01s_Init
*	����˵��: bsp_esp01s_Init�����ӿڳ�ʼ��
*	��    ��: ��
*	�� �� ֵ: ��
*   ��    ע��
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
*	�� �� ��: esp01s_Init
*	����˵��: esp_01s�ϵ��ʼ��
*	��    ��: ��
*	�� �� ֵ: ��
*   ��    ע��
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
    esp_01s_TxData("AT+CIPMUX=1\r\n", strlen("AT+CIPMUX=1\r\n")); // �ϵ���Ҫ���·���ָ����ֻ����Ӷ˿�
    delay_ms(100);
    esp_01s_TxData("AT+CIPSERVER=1,8080\r\n", strlen("AT+CIPSERVER=1,8080\r\n")); // �ֻ����Ӷ˿�����
    //    delay_ms(1000);
    //    esp_01s_TxData("AT+CIFSR\r\n",strlen("AT+CIFSR\r\n"));
    //    delay_ms(1000);
    //    if(strcmp(esp01s_RxData, "OK") != 0)
    //      return HAL_ERROR;

    return HAL_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: esp_01s_TxData
*	����˵��: esp_01s��������
*	��    ��: ��
*	�� �� ֵ: ��
*   ��    ע��
*********************************************************************************************************
*/
void esp_01s_TxData(uint8_t *_DataBuf, uint32_t _Len)
{
    HAL_UART_Transmit_DMA(&huart1, _DataBuf, _Len);
}

/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: 
*	��    ��: 
*	�� �� ֵ: 
*   ��    ע��
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
*	�� �� ��: 
*	����˵��: 
*	��    ��: 
*	�� �� ֵ: 
*   ��    ע��
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