#include "perf_counter_configure.h"
#include "perf_counter.h"

#define isUsed 0 //标记用户是否使用过systick

//extern unsigned int system_core_clock;

/**
 * @brief  系统滴答定时器初始化，定时1ms
 * @param  无
 * @retval 无
 */
void systickConfigure(void)
{
//	SystemCoreClock = system_core_clock;
#if isUsed
    init_cycle_counter(true);
#else
    init_cycle_counter(false); //如果你的系统没有使用systick,则传入false
#endif
}

/**
 * @brief  获取单片机自上电以来经过的微秒数
 * @param  无
 * @retval 当前系统时钟微秒数
 */
uint32_t micros(void)
{
    return get_system_us();
}

/**
 * @brief  获取单片机自上电以来经过的毫秒数
 * @param  无
 * @retval 当前系统时钟毫秒数
 */
uint32_t millis(void)
{
    return get_system_ms();
}

