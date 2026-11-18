#ifndef GARAGE_TASK_MONITOR_H
#define GARAGE_TASK_MONITOR_H

#include "FreeRTOS.h"
#include "task.h"

namespace garage
{

class TaskMonitor
{
public:

    float cpuPercent;           // 最近一次CPU占用率
    uint32_t minFreeStackBytes; // 任务从创建以来，历史最少还剩多少字节栈空间
    uint32_t loopCount;         // 调用次数
    TickType_t lastLoopTick;    // 最近一次调用update()时的FreeRTOS系统节拍
    bool cpuValid;              // 数据是否有效

    TaskMonitor()
        : cpuPercent(0.0f), minFreeStackBytes(0), loopCount(0),
          lastLoopTick(0), cpuValid(false), _task(0),
          _previousTime(0), _previousRunTime(0)
    {
    }

    void begin()
    {
        _task = xTaskGetCurrentTaskHandle();
        _previousTime = portGET_RUN_TIME_COUNTER_VALUE();
        _previousRunTime = ulTaskGetRunTimeCounter(_task);
        cpuValid = false;
    }

    /***********************************************************************
    ** 函 数 名： void TaskMonitor::update()
    ** 功能说明： 每轮记录任务心跳，约每秒更新一次CPU和栈统计
    ** 调用位置： 对应任务的循环中，begin()必须先执行
    ***********************************************************************/
    void update()
    {
        if (_task == 0)
        {
            begin();
        }

        ++loopCount;
        lastLoopTick = xTaskGetTickCount();
        // 读取当前TIM2总计数
        const configRUN_TIME_COUNTER_TYPE now = portGET_RUN_TIME_COUNTER_VALUE();
        // 计算这段时间内TIM2走过的计数
        const configRUN_TIME_COUNTER_TYPE elapsed = now - _previousTime;

        // TASK_STATS_TIMER_HZ在工程里是100000。
        // 正常计数时，不满100000个计数就提前返回，等下一轮再检查。
        if (now >= _previousTime && elapsed < TASK_STATS_TIMER_HZ)
        {
            return;
        }

        // 读取FreeRTOS已给这个任务累计的运行计数
        const configRUN_TIME_COUNTER_TYPE runTime = ulTaskGetRunTimeCounter(_task);

        // 两个计数都没有倒退，才用这组数据计算百分比
        cpuValid = now >= _previousTime && runTime >= _previousRunTime;

        if (cpuValid)
        {
            const configRUN_TIME_COUNTER_TYPE used = runTime - _previousRunTime;
            cpuPercent = 100.0f * static_cast<float>(used) / static_cast<float>(elapsed);
        }

        minFreeStackBytes = uxTaskGetStackHighWaterMark(_task) * sizeof(StackType_t);

        _previousTime = now;
        _previousRunTime = runTime;
    }

private:
    TaskHandle_t _task;
    configRUN_TIME_COUNTER_TYPE _previousTime;
    configRUN_TIME_COUNTER_TYPE _previousRunTime;
};

}

#endif
