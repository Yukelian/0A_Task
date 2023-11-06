#ifndef __MILLISTASKMANAGER_H
#define __MILLISTASKMANAGER_H

#include "stdint.h"
#include "stdbool.h"

typedef void (*TaskFunction_t)(void); //任务回调函数

struct Task
{
    bool State;              //任务状态
    TaskFunction_t Function; //任务函数指针
    uint32_t Time;           //任务时间
    uint32_t TimePrev;       //任务上一次触发时间
    uint32_t TimeCost;       //任务时间开销(us)
    uint32_t TimeError;      //误差时间
    struct Task *Next;       //下一个节点
};

typedef struct Task Task_t; //任务类型定义

Task_t *Register(TaskFunction_t func, uint32_t timeMs, bool state);
Task_t *Find(TaskFunction_t func);
Task_t *GetPrev(Task_t *task);
bool Logout(TaskFunction_t func);
bool SetState(TaskFunction_t func, bool state);
bool SetIntervalTime(TaskFunction_t func, uint32_t timeMs);
uint32_t GetTimeCost(TaskFunction_t func);
uint32_t GetTickElaps(uint32_t nowTick, uint32_t prevTick);
void Running(uint32_t tick);
float GetCPU_Usage(void);

void taskManagerBeging(bool priorityEnable);

#endif
