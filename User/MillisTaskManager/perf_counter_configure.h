#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdint.h>
#include <stdbool.h>
#include "perf_counter.h"

extern uint32_t SystemCoreClock;

#define get_system_ms() (get_system_ticks() / (SystemCoreClock / 1000ul))
#define get_system_us() (get_system_ticks() / (SystemCoreClock / 1000000ul))
#define get_nCycle_to_us(xx) (xx / (SystemCoreClock / 1000000ul))
#define get_nCycle_to_ms(xx) (xx / (SystemCoreClock / 1000ul))


void systickConfigure(void);
uint32_t micros(void);
uint32_t millis(void);

#endif

