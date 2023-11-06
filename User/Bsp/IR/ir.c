
void IR_Input_Choose1(void)
{
    uint16_t dval; /* �½���ʱ��������ֵ */

    if (RDATA) /* �����ز��� */
    {          /* һ��Ҫ�����ԭ�������� */
        TIM_RESET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1);
        TIM_SET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1,
                                TIM_ICPOLARITY_FALLING); /* ����Ϊ�½��ز��� */
        __HAL_TIM_SET_COUNTER(&g_tim1_handle, 0);        /* ��ն�ʱ��ֵ */
        g_remote_sta |= 0X10;                            /* ����������Ѿ������� */
    }
    else /* �½��ز��� */
    {    /* ��ȡCCR1Ҳ������CC1IF��־λ */
        dval = HAL_TIM_ReadCapturedValue(&g_tim1_handle, TIM_CHANNEL_1);
        /* һ��Ҫ�����ԭ�������� */
        TIM_RESET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1);
        TIM_SET_CAPTUREPOLARITY(&g_tim1_handle, TIM_CHANNEL_1,
                                TIM_ICPOLARITY_RISING); /* ����TIM5ͨ��1�����ز��� */

        if (g_remote_sta & 0X10) /* ���һ�θߵ�ƽ���� */
        {
            if (g_remote_sta & 0X80) /* ���յ��������� */
            {
                if (dval > 300 && dval < 800) /* 560Ϊ��׼ֵ,560us */
                {
                    g_remote_data >>= 1;          /* ����һλ. */
                    g_remote_data &= ~0x80000000; /* ���յ�0 */
                }
                else if (dval > 1400 && dval < 1800) /* 1680Ϊ��׼ֵ,1680us */
                {
                    g_remote_data >>= 1;         /* ����һλ*/
                    g_remote_data |= 0x80000000; /* ���յ�1 */
                }
                else if (dval > 2000 && dval < 3000)
                {                         /* �õ�������ֵ���ӵ���Ϣ 2250Ϊ��׼ֵ2.25ms */
                    g_remote_cnt++;       /* ������������1�� */
                    g_remote_sta &= 0XF0; /* ��ռ�ʱ�� */
                }
            }
            else if (dval > 4200 && dval < 4700) /* 4500Ϊ��׼ֵ4.5ms */
            {
                g_remote_sta |= 1 << 7; /* ��ǳɹ����յ��������� */
                g_remote_cnt = 0;       /* ����������������� */
            }
        }
        g_remote_sta &= ~(1 << 4);
    }

    void IR_Input_Choose2(void)
    {
        if (g_remote_sta & 0x80) /* �ϴ������ݱ����յ��� */
        {
            g_remote_sta &= ~0X10; /* ȡ���������Ѿ��������� */

            if ((g_remote_sta & 0X0F) == 0X00)
            {
                g_remote_sta |= 1 << 6; /* ����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ� */
            }

            if ((g_remote_sta & 0X0F) < 14)
            {
                g_remote_sta++;
            }
            else
            {
                g_remote_sta &= ~(1 << 7); /* ���������ʶ */
                g_remote_sta &= 0XF0;      /* ��ռ����� */
            }
        }
    }
}

/**
 * @brief     	�������ⰴ��(���ư���ɨ��)
 * @param     	��
 * @retval    	0   , û���κΰ�������
 *              	����, ���µİ�����ֵ
 */
uint8_t remote_scan(void)
{
    uint8_t sta = 0;
    uint8_t t1, t2;

    if (g_remote_sta & (1 << 6)) /* �õ�һ��������������Ϣ�� */
    {
        t1 = g_remote_data;               /* �õ���ַ�� */
        t2 = (g_remote_data >> 8) & 0xff; /* �õ���ַ���� */

        if ((t1 == (uint8_t)~t2) && t1 == REMOTE_ID)
        { /* ����ң��ʶ����(ID)����ַ */
            t1 = (g_remote_data >> 16) & 0xff;
            t2 = (g_remote_data >> 24) & 0xff;

            if (t1 == (uint8_t)~t2)
            {
                sta = t1; /* ��ֵ��ȷ */
            }
        }

        if ((sta == 0) || ((g_remote_sta & 0X80) == 0))
        {                              /* �������ݴ���/ң���Ѿ�û�а����� */
            g_remote_sta &= ~(1 << 6); /* ������յ���Ч������ʶ */
            g_remote_cnt = 0;          /* ����������������� */
        }
    }

    return sta;
}
