#ifndef GARAGE_BATT_PUSH_TASK_H
#define GARAGE_BATT_PUSH_TASK_H

#include "TaskMonitor.h"
#include "Libraries/Can/C610Can.h"
#include "Libraries/Motor/M2006.h"
#include "Libraries/Math/Pid/PidController.h"

namespace garage
{

// 电池推送任务：通过 CAN2 控制 C610+M2006 电机转到指定角度。
// 上电首次收到反馈时，把当前位置记为 0°，之后所有角度相对该零位。
class BattPushTask
{
public:
    BattPushTask();
    bool init(FDCAN_HandleTypeDef &hfdcan);
    void update();

    void requestAngle(float deg); // 设置目标角度并使能位置闭环

    volatile float targetAngleDeg;  // 目标角度（相对上电零位，输出轴 0~360），Watch 直接改
    volatile float currentAngleDeg; // 当前角度（相对上电零位，输出轴 0~360），Watch 里看
    volatile bool  enabled;         // 位置闭环使能，上电自动为 1，Watch 置 0 可停止

    TaskMonitor taskMonitor;
    C610Can can;
    M2006 motor;
    PidController pid;

private:
    uint32_t _lastUpdateMs;
    float _appliedAngle; // 已应用的角度，检测 Watch 是否改了目标
};

}

#endif
