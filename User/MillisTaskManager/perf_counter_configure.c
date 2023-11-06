#include "perf_counter_configure.h"
#include "perf_counter.h"

#define isUsed 0 //����û��Ƿ�ʹ�ù�systick

//extern unsigned int system_core_clock;

/**
 * @brief  ϵͳ�δ�ʱ����ʼ������ʱ1ms
 * @param  ��
 * @retval ��
 */
void systickConfigure(void)
{
//	SystemCoreClock = system_core_clock;
#if isUsed
    init_cycle_counter(true);
#else
    init_cycle_counter(false); //������ϵͳû��ʹ��systick,����false
#endif
}

/**
 * @brief  ��ȡ��Ƭ�����ϵ�����������΢����
 * @param  ��
 * @retval ��ǰϵͳʱ��΢����
 */
uint32_t micros(void)
{
    return get_system_us();
}

/**
 * @brief  ��ȡ��Ƭ�����ϵ����������ĺ�����
 * @param  ��
 * @retval ��ǰϵͳʱ�Ӻ�����
 */
uint32_t millis(void)
{
    return get_system_ms();
}

