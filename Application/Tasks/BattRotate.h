#ifndef GARAGE_BATT_ROTATE_TASK_H
#define GARAGE_BATT_ROTATE_TASK_H

#include "TaskMonitor.h"
#include "Libraries/Can/Gm6020Can.h"
#include "Libraries/Motor/Gm6020.h"
#include "Libraries/Math/Pid/PidController.h"
#include "Libraries/Serial/UartLineReceiver.h"

namespace garage
{

// 电池旋转任务：通过 CAN1 控制 GM6020 电机转到指定角度。
// 上电首次收到反馈时，把当前位置记为 0°，之后所有角度都相对该零位。
// 串口（USART6）发送 "R90" 转到 90°（相对零位），"S" 停止并卸载。
class BattRotateTask
{
public:
    BattRotateTask();
    bool init(FDCAN_HandleTypeDef &hfdcan, UART_HandleTypeDef &huart);
    void update();

    void requestAngle(float deg); // 设置目标角度并使能位置闭环

    volatile float targetAngleDeg;  // 目标角度（相对上电零位，0~360），Watch 直接改
    volatile float currentAngleDeg; // 当前角度（相对上电零位，0~360），Watch 里看
    volatile bool  enabled;         // 位置闭环使能，上电自动为 1，Watch 置 0 可停止

    TaskMonitor taskMonitor;
    Gm6020Can can;
    Gm6020 motor;
    PidController pid;

private:
    void handleCommand(const char *line);

    UartLineReceiver _serial;
    uint32_t _lastUpdateMs;
    float _appliedAngle;    // 已应用的角度，检测 Watch 是否改了目标
    uint16_t _zeroAngleRaw; // 上电零位（编码器原始值）
    bool _zeroed;           // 是否已完成零位标定
};

}

#endif
