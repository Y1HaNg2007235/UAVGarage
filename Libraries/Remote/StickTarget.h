#ifndef GARAGE_STICK_TARGET_H
#define GARAGE_STICK_TARGET_H

#include <stdint.h>

namespace garage
{

struct StickSettings
{
    uint16_t low;
    uint16_t center;
    uint16_t high;
    uint16_t deadband;
    bool reversed;
    float maxTargetRateDegS; // Rate of the existing position target, not a new speed PID.
    float maxLeadDeg;       // Keep error below half a turn for the existing wrapDeg PID.
};

class StickTarget
{
public:
    explicit StickTarget(const StickSettings &settings);
    float update(uint16_t channel, bool linkUsable, bool axisReady,
                 float positionDeg, float dtSeconds);
    void reset();
    bool active() const { return _active; }
    float target() const { return _target; }
    float demand() const { return _demand; }

private:
    StickSettings _settings;
    bool _active;
    bool _moving;
    float _target;
    float _demand;
};

}
#endif
