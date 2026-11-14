#include "BattPush.h"
#include "main.h"

namespace garage
{
namespace
{
// 位置环增益，角度误差单位度，输出单位电流给定。M2006 输出轴慢，增益相应小一点。
const float POSITION_KP = 80.0f;
const float POSITION_KI = 2.0f;
const float POSITION_KD = 5.0f;

// C610 电流给定范围 -10000~10000（对应 -10A~10A）
const float MAX_OUTPUT = 10000.0f;
const float INTEGRAL_LIMIT = 3000.0f;

/***********************************************************************
** 函 数 名： wrapDeg(float deg)
** 函数说明： 把角度误差回绕到 -180~180
**---------------------------------------------------------------------
** 输入参数： deg，任意角度差
** 返回参数： 最短路径的角度差
***********************************************************************/
float wrapDeg(float deg)
{
    while (deg > 180.0f)
    {
        deg -= 360.0f;
    }
    while (deg < -180.0f)
    {
        deg += 360.0f;
    }
    return deg;
}

/***********************************************************************
** 函 数 名： normalize360(float deg)
** 函数说明： 把角度归一化到 0~360
**---------------------------------------------------------------------
** 输入参数： deg，任意角度
** 返回参数： 0~360 的角度
***********************************************************************/
float normalize360(float deg)
{
    while (deg >= 360.0f)
    {
        deg -= 360.0f;
    }
    while (deg < 0.0f)
    {
        deg += 360.0f;
    }
    return deg;
}
}

BattPushTask::BattPushTask()
    : targetAngleDeg(0.0f), currentAngleDeg(0.0f), enabled(true),
      motor(1), _lastUpdateMs(0), _appliedAngle(0.0f)
{
}

/***********************************************************************
** 函 数 名： BattPushTask::init(FDCAN_HandleTypeDef &hfdcan)
** 函数说明： 初始化 CAN 和位置环
**---------------------------------------------------------------------
** 输入参数： hfdcan，CAN2 句柄
** 返回参数： true 表示初始化成功
***********************************************************************/
bool BattPushTask::init(FDCAN_HandleTypeDef &hfdcan)
{
    if (!can.init(hfdcan))
    {
        return false;
    }

    pid.setGains(POSITION_KP, POSITION_KI, POSITION_KD);
    pid.setOutputLimit(MAX_OUTPUT);
    pid.setIntegralLimit(INTEGRAL_LIMIT);
    pid.reset();
    return true;
}

/***********************************************************************
** 函 数 名： BattPushTask::requestAngle(float deg)
** 函数说明： 归一化角度并启动位置闭环
**---------------------------------------------------------------------
** 输入参数： deg，目标角度（任意值，归一化到 0~360）
** 返回参数： 无
***********************************************************************/
void BattPushTask::requestAngle(float deg)
{
    targetAngleDeg = normalize360(deg);
    _appliedAngle = targetAngleDeg;
    pid.reset();
    enabled = true;
}

/***********************************************************************
** 函 数 名： BattPushTask::update()
** 函数说明： 接收反馈、执行位置闭环并发送电流给定
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void BattPushTask::update()
{
    taskMonitor.update();
    can.update(); // 接收反馈 + 发送控制帧

    // 把 CAN 反馈同步到电机对象
    C610Can::Feedback fb;
    if (can.readFeedback(motor.motorId, fb))
    {
        motor.updateFeedback(fb.angle, fb.speedRpm, fb.torqueCurrent, fb.valid);
    }
    currentAngleDeg = motor.angleDegValue;

    // 位置闭环
    int16_t output = 0;
    if (enabled && motor.feedbackValid)
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

}
