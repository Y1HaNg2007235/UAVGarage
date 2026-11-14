#include "PidController.h"

namespace garage
{

PidController::PidController()
    : output(0.0f), _kp(0.0f), _ki(0.0f), _kd(0.0f),
      _outputLimit(1.0f), _integralLimit(0.0f), _integral(0.0f),
      _lastError(0.0f), _first(true)
{
}

/***********************************************************************
** 函 数 名： PidController::setGains(float kp, float ki, float kd)
** 函数说明： 设置 PID 三个增益
**---------------------------------------------------------------------
** 输入参数： kp、ki、kd，比例/积分/微分增益
** 返回参数： 无
***********************************************************************/
void PidController::setGains(float kp, float ki, float kd)
{
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

/***********************************************************************
** 函 数 名： PidController::setOutputLimit(float limit)
** 函数说明： 设置输出对称限幅
**---------------------------------------------------------------------
** 输入参数： limit，输出限幅绝对值
** 返回参数： 无
***********************************************************************/
void PidController::setOutputLimit(float limit)
{
    _outputLimit = limit > 0.0f ? limit : 0.0f;
}

/***********************************************************************
** 函 数 名： PidController::setIntegralLimit(float limit)
** 函数说明： 设置积分项对称限幅，用于抗积分饱和
**---------------------------------------------------------------------
** 输入参数： limit，积分项限幅绝对值，0 表示不限幅
** 返回参数： 无
***********************************************************************/
void PidController::setIntegralLimit(float limit)
{
    _integralLimit = limit >= 0.0f ? limit : 0.0f;
}

/***********************************************************************
** 函 数 名： PidController::reset()
** 函数说明： 清空积分项和历史误差
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void PidController::reset()
{
    _integral = 0.0f;
    _lastError = 0.0f;
    _first = true;
    output = 0.0f;
}

/***********************************************************************
** 函 数 名： PidController::update(float error, float dt)
** 函数说明： 计算一次 PID 输出，包含输出与积分限幅
**---------------------------------------------------------------------
** 输入参数： error，误差；dt，距上次更新的时间（秒）
** 返回参数： 限幅后的输出
***********************************************************************/
float PidController::update(float error, float dt)
{
    if (dt <= 0.0f)
    {
        return output;
    }

    const float derivative = _first ? 0.0f : (error - _lastError) / dt;
    _first = false;

    _integral += error * dt;
    if (_integralLimit > 0.0f)
    {
        if (_integral > _integralLimit)
        {
            _integral = _integralLimit;
        }
        else if (_integral < -_integralLimit)
        {
            _integral = -_integralLimit;
        }
    }

    float result = _kp * error + _ki * _integral + _kd * derivative;

    if (result > _outputLimit)
    {
        result = _outputLimit;
    }
    else if (result < -_outputLimit)
    {
        result = -_outputLimit;
    }

    _lastError = error;
    output = result;
    return result;
}

}
