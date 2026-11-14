#include "BattRotate.h"
#include "main.h"
#include "Libraries/Math/AngleUtils.h"
#include <stdlib.h>

namespace garage
{
namespace
{
// 位置环增益，角度误差单位度，输出单位电压给定。先给 P 起步，再按实测调。
const float POSITION_KP = 200.0f;
const float POSITION_KI = 25.0f;
const float POSITION_KD = 2.0f;

// GM6020 默认电压控制模式，给定范围 -25000~25000。
// 若已在 RoboMaster Assistant 打开电流环，改为 16384。
const float MAX_OUTPUT = 25000.0f;
const float INTEGRAL_LIMIT = 5000.0f;
}

BattRotateTask::BattRotateTask()
    : targetAngleDeg(0.0f), currentAngleDeg(0.0f), enabled(true),
      motor(1), _lastUpdateMs(0), _appliedAngle(0.0f), _zeroAngleRaw(0), _zeroed(false)
{
}

/***********************************************************************
** 函 数 名： BattRotateTask::init(...)
** 函数说明： 初始化 CAN、串口和位置环
**---------------------------------------------------------------------
** 输入参数： hfdcan，CAN1 句柄；huart，指令串口句柄
** 返回参数： true 表示初始化成功
***********************************************************************/
bool BattRotateTask::init(FDCAN_HandleTypeDef &hfdcan, UART_HandleTypeDef &huart)
{
    if (!can.init(hfdcan))
    {
        return false;
    }
    _serial.init(huart);

    pid.setGains(POSITION_KP, POSITION_KI, POSITION_KD);
    pid.setOutputLimit(MAX_OUTPUT);
    pid.setIntegralLimit(INTEGRAL_LIMIT);
    pid.reset();
    return true;
}

/***********************************************************************
** 函 数 名： BattRotateTask::requestAngle(float deg)
** 函数说明： 归一化角度并启动位置闭环
**---------------------------------------------------------------------
** 输入参数： deg，目标角度（任意值，归一化到 0~360）
** 返回参数： 无
***********************************************************************/
void BattRotateTask::requestAngle(float deg)
{
    targetAngleDeg = normalize360(deg);
    _appliedAngle = targetAngleDeg;
    pid.reset();
    enabled = true;
}

/***********************************************************************
** 函 数 名： BattRotateTask::update()
** 函数说明： 接收反馈、解析指令、执行位置闭环并发送给定
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void BattRotateTask::update()
{
    taskMonitor.update();
    can.update(); // 接收反馈 + 发送控制帧

    // 把 CAN 反馈同步到电机对象
    Gm6020Can::Feedback fb;
    if (can.readFeedback(motor.motorId, fb))
    {
        motor.updateFeedback(fb.angle, fb.speedRpm, fb.torqueCurrent, fb.temperature, fb.valid);
    }

    // 上电首次收到反馈时，把当前位置记为 0°
    if (!_zeroed && motor.feedbackValid)
    {
        _zeroAngleRaw = motor.angleRaw;
        _zeroed = true;
        targetAngleDeg = 0.0f;
        _appliedAngle = 0.0f;
        pid.reset();
    }

    // 计算相对上电零位的当前角度（0~360）
    int32_t delta = static_cast<int32_t>(motor.angleRaw) - static_cast<int32_t>(_zeroAngleRaw);
    if (delta < 0)
    {
        delta += Gm6020::ENCODER_RESOLUTION;
    }
    currentAngleDeg = Gm6020::countToDeg(static_cast<uint16_t>(delta));

    // 解析串口指令
    char line[UartLineReceiver::LINE_BUFFER_SIZE];
    if (_serial.readLine(line, sizeof(line)))
    {
        handleCommand(line);
    }

    // 位置闭环
    int16_t output = 0;
    if (enabled && motor.feedbackValid && _zeroed)
    {
        // Watch 里直接改了 targetAngleDeg 时，归一化并清 PID 重新收敛
        const float target = normalize360(targetAngleDeg);
        if (target != _appliedAngle)
        {
            _appliedAngle = target;
            pid.reset();
        }

        const uint32_t now = HAL_GetTick();
        const float dt = static_cast<float>(now - _lastUpdateMs) * 0.001f;
        _lastUpdateMs = now;

        const float error = wrapDeg(target - currentAngleDeg);
        output = static_cast<int16_t>(pid.update(error, dt));
    }
    else
    {
        pid.reset();
        output = 0;
    }

    motor.setOutput(output);
    can.setMotorOutput(motor.motorId, output);
}

/***********************************************************************
** 函 数 名： BattRotateTask::handleCommand(const char *line)
** 函数说明： 解析 "R<角度>" 和 "S" 指令
**---------------------------------------------------------------------
** 输入参数： line，去掉换行的指令行
** 返回参数： 无
***********************************************************************/
void BattRotateTask::handleCommand(const char *line)
{
    if (line[0] == 'R' || line[0] == 'r')
    {
        const float deg = static_cast<float>(atof(line + 1));
        requestAngle(deg);
    }
    else if (line[0] == 'S' || line[0] == 's')
    {
        enabled = false;
        pid.reset();
    }
}

}
