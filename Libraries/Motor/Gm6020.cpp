#include "Gm6020.h"
#include "main.h"

namespace garage
{

Gm6020::Gm6020(uint8_t id)
    : motorId(id), output(0), angleRaw(0), speedRpm(0), torqueCurrent(0),
      temperature(0), feedbackValid(false), feedbackMs(0), angleDegValue(0.0f)
{
}

/***********************************************************************
** 函 数 名： Gm6020::setOutput(int16_t output)
** 函数说明： 记录给定值，实际发送由 CAN 层统一打包
**---------------------------------------------------------------------
** 输入参数： output，电流/电压给定值
** 返回参数： 无
***********************************************************************/
void Gm6020::setOutput(int16_t out)
{
    output = out;
}

/***********************************************************************
** 函 数 名： Gm6020::updateFeedback(...)
** 函数说明： 用 CAN 反馈帧刷新电机状态
**---------------------------------------------------------------------
** 输入参数： 角度、转速、电流、温度、是否有效
** 返回参数： 无
***********************************************************************/
void Gm6020::updateFeedback(uint16_t angle, int16_t speed, int16_t torque,
                            int8_t temp, bool valid)
{
    angleRaw = angle;
    speedRpm = speed;
    torqueCurrent = torque;
    temperature = temp;
    feedbackValid = valid;
    feedbackMs = HAL_GetTick();
    angleDegValue = countToDeg(angle);
}

/***********************************************************************
** 函 数 名： Gm6020::angleDeg() const
** 函数说明： 机械角度换算成度
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 0~360 的角度
***********************************************************************/
float Gm6020::angleDeg() const
{
    return countToDeg(angleRaw);
}

float Gm6020::countToDeg(uint16_t count)
{
    return static_cast<float>(count) * 360.0f / static_cast<float>(ENCODER_RESOLUTION);
}

uint16_t Gm6020::degToCount(float deg)
{
    while (deg >= 360.0f)
    {
        deg -= 360.0f;
    }
    while (deg < 0.0f)
    {
        deg += 360.0f;
    }
    return static_cast<uint16_t>(deg * static_cast<float>(ENCODER_RESOLUTION) / 360.0f);
}

}
