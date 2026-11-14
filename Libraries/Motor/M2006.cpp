#include "M2006.h"
#include "main.h"

namespace garage
{

M2006::M2006(uint8_t id)
    : motorId(id), output(0), angleRaw(0), speedRpm(0), torqueCurrent(0),
      feedbackValid(false), feedbackMs(0), angleDegValue(0.0f),
      _rotorAccum(0), _lastRotorRaw(0), _first(true)
{
}

/***********************************************************************
** 函 数 名： M2006::setOutput(int16_t output)
** 函数说明： 记录电流给定，实际发送由 CAN 层统一打包
**---------------------------------------------------------------------
** 输入参数： output，电流给定 -10000~10000
** 返回参数： 无
***********************************************************************/
void M2006::setOutput(int16_t out)
{
    output = out;
}

/***********************************************************************
** 函 数 名： M2006::updateFeedback(...)
** 函数说明： 累计转子角度并换算输出轴单圈角度
**---------------------------------------------------------------------
** 输入参数： 转子角度、转速、转矩、是否有效
** 返回参数： 无
***********************************************************************/
void M2006::updateFeedback(uint16_t rotorAngle, int16_t speed, int16_t torque, bool valid)
{
    angleRaw = rotorAngle;
    speedRpm = speed;
    torqueCurrent = torque;
    feedbackValid = valid;
    feedbackMs = HAL_GetTick();

    // 累计转子角度（处理 0~8191 回绕）
    if (_first)
    {
        _first = false;
        _lastRotorRaw = rotorAngle;
        _rotorAccum = 0;
    }
    else
    {
        int32_t delta = static_cast<int32_t>(rotorAngle) - static_cast<int32_t>(_lastRotorRaw);
        if (delta > 4096)
        {
            delta -= ENCODER_RESOLUTION;
        }
        else if (delta < -4096)
        {
            delta += ENCODER_RESOLUTION;
        }
        _rotorAccum += delta;
        _lastRotorRaw = rotorAngle;
    }

    // 输出轴单圈角度（0~360）
    int64_t accum = _rotorAccum % OUTPUT_RESOLUTION;
    if (accum < 0)
    {
        accum += OUTPUT_RESOLUTION;
    }
    angleDegValue = static_cast<float>(accum) * 360.0f /
                    static_cast<float>(OUTPUT_RESOLUTION);
}

/***********************************************************************
** 函 数 名： M2006::setZero()
** 函数说明： 清空累计，把当前位置记为 0°
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void M2006::setZero()
{
    _rotorAccum = 0;
    _lastRotorRaw = angleRaw;
    angleDegValue = 0.0f;
}

}
