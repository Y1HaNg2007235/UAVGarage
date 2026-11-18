#include "RemoteControl.h"
#include "RemoteConfig.h"
#include "Vehicle/Garage/Garage.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

extern garage::Garage robot;
garage::RemoteControlTask remoteControl;

namespace garage
{

RemoteControlTask::RemoteControlTask()
    : enabled(true), initialized(false), online(false), frames(0), frameLost(false),
      failsafe(false), gmDemand(0), m2006Demand(0), gmTargetDeg(0), m2006TargetDeg(0),
      _gmTarget(remote_config::GM6020_STICK), _mTarget(remote_config::M2006_STICK), _lastTargetMs(0)
{
    for (unsigned i = 0; i < 16; ++i) { channels[i] = 0; }
}

void RemoteControlTask::init()
{
    initialized = receiver.init(huart5);
    _lastTargetMs = HAL_GetTick();
}

void RemoteControlTask::update()
{
    taskMonitor.update();
    receiver.poll();
    initialized = receiver.ready;
    const uint32_t now = HAL_GetTick();
    const SbusDecoder::Frame &frame = receiver.decoder.frame();
    if (frame.sequence != frames)
    {
        for (unsigned i = 0; i < 16; ++i) { channels[i] = frame.channels[i]; }
        frameLost = frame.frameLost;
        failsafe = frame.failsafe;
        frames = frame.sequence;
    }
    online = receiver.ready && receiver.decoder.usable(now, remote_config::LINK_TIMEOUT_MS);
    const uint32_t elapsed = now - _lastTargetMs;
    if (elapsed < remote_config::TARGET_PERIOD_MS) { return; }
    _lastTargetMs = now;
    if (!enabled)
    {
        _gmTarget.reset(); _mTarget.reset(); gmDemand = m2006Demand = 0;
        return; // Preserve the existing Watch and USART6 command behavior.
    }

    Gm6020Can::Feedback gmFeedback;
    C610Can::Feedback mFeedback;
    bool gmReady, mReady;
    float gmAngle, mAngle;
    // Both original motor tasks read CAN in task context; take a consistent snapshot.
    taskENTER_CRITICAL();
    gmReady = robot.battRotate.enabled && robot.battRotate.motor.feedbackValid &&
        robot.battRotate.can.readFeedback(robot.battRotate.motor.motorId, gmFeedback) &&
        gmFeedback.valid && static_cast<uint32_t>(now - gmFeedback.receivedMs) <= remote_config::MOTOR_FEEDBACK_TIMEOUT_MS;
    mReady = robot.battPush.enabled && robot.battPush.motor.feedbackValid &&
        robot.battPush.can.readFeedback(robot.battPush.motor.motorId, mFeedback) &&
        mFeedback.valid && static_cast<uint32_t>(now - mFeedback.receivedMs) <= remote_config::MOTOR_FEEDBACK_TIMEOUT_MS;
    gmAngle = robot.battRotate.currentAngleDeg;
    mAngle = robot.battPush.currentAngleDeg;
    taskEXIT_CRITICAL();

    const unsigned gmChannel = remote_config::GM6020_CHANNEL;
    const unsigned mChannel = remote_config::M2006_CHANNEL;
    const bool channelsValid = gmChannel >= 1U && gmChannel <= 16U && mChannel >= 1U && mChannel <= 16U;
    const float dt = static_cast<float>(elapsed) * 0.001f;
    gmTargetDeg = _gmTarget.update(channelsValid ? frame.channels[gmChannel - 1U] : 0,
        online && channelsValid, gmReady, gmAngle, dt);
    m2006TargetDeg = _mTarget.update(channelsValid ? frame.channels[mChannel - 1U] : 0,
        online && channelsValid, mReady, mAngle, dt);
    gmDemand = _gmTarget.demand(); m2006Demand = _mTarget.demand();

    taskENTER_CRITICAL();
    if (_gmTarget.active() && robot.battRotate.enabled) { robot.battRotate.targetAngleDeg = gmTargetDeg; }
    if (_mTarget.active() && robot.battPush.enabled) { robot.battPush.targetAngleDeg = m2006TargetDeg; }
    taskEXIT_CRITICAL();
}

}

extern "C" void RemoteControl_Init(void) { remoteControl.init(); }
extern "C" void RemoteControl_Update(void) { remoteControl.update(); }
