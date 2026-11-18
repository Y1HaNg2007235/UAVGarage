#include "StickTarget.h"
#include <float.h>
#include <math.h>

namespace garage
{
namespace
{
bool finiteValue(float value) { return value >= -FLT_MAX && value <= FLT_MAX; }
float clampValue(float value, float low, float high)
{
    return value < low ? low : (value > high ? high : value);
}
float normalize(float angle)
{
    angle = fmodf(angle, 360.0f);
    return angle < 0.0f ? angle + 360.0f : angle;
}
float difference(float angle)
{
    angle = normalize(angle);
    return angle > 180.0f ? angle - 360.0f : angle;
}
}

StickTarget::StickTarget(const StickSettings &settings)
    : _settings(settings), _active(false), _moving(false), _target(0), _demand(0)
{
}

void StickTarget::reset()
{
    _active = false;
    _moving = false;
    _demand = 0;
}

float StickTarget::update(uint16_t channel, bool linkUsable, bool axisReady,
                         float positionDeg, float dtSeconds)
{
    const bool validSettings = _settings.center > _settings.low && _settings.high > _settings.center &&
        _settings.deadband < _settings.center - _settings.low &&
        _settings.deadband < _settings.high - _settings.center &&
        finiteValue(_settings.maxTargetRateDegS) && _settings.maxTargetRateDegS > 0 &&
        finiteValue(_settings.maxLeadDeg) && _settings.maxLeadDeg > 0 && _settings.maxLeadDeg < 180;
    if (!axisReady || !finiteValue(positionDeg) || !validSettings)
    {
        reset();
        return _target;
    }
    positionDeg = normalize(positionDeg);
    if (!linkUsable)
    {
        // Set a single hold target on loss; do not chase measured position every cycle.
        if (_active && _moving) { _target = positionDeg; }
        _moving = false;
        _demand = 0;
        return _target;
    }
    if (!_active)
    {
        _target = positionDeg;
        _active = true;
    }
    const int error = static_cast<int>(channel) - _settings.center;
    const int deadband = _settings.deadband;
    if (error > deadband)
    {
        _demand = static_cast<float>(error - deadband) /
            (_settings.high - _settings.center - deadband);
    }
    else if (error < -deadband)
    {
        _demand = static_cast<float>(error + deadband) /
            (_settings.center - _settings.low - deadband);
    }
    else { _demand = 0; }
    _demand = clampValue(_demand, -1, 1);
    if (_settings.reversed) { _demand = -_demand; }
    if (_demand == 0)
    {
        if (_moving) { _target = positionDeg; }
        _moving = false;
        return _target;
    }
    if (!finiteValue(dtSeconds) || dtSeconds <= 0 || dtSeconds > 0.05f)
    {
        _target = positionDeg;
        _moving = false;
        _demand = 0;
        return _target;
    }
    _moving = true;
    const float next = _target + _demand * _settings.maxTargetRateDegS * dtSeconds;
    const float lead = clampValue(difference(next - positionDeg), -_settings.maxLeadDeg, _settings.maxLeadDeg);
    _target = normalize(positionDeg + lead);
    return _target;
}

}
