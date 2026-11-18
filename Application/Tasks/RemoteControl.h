#ifndef GARAGE_REMOTE_CONTROL_H
#define GARAGE_REMOTE_CONTROL_H

#ifdef __cplusplus
#include "Libraries/Remote/SbusUartReceiver.h"
#include "Libraries/Remote/StickTarget.h"
#include "Application/TaskMonitor.h"

namespace garage
{
class RemoteControlTask
{
public:
    RemoteControlTask();
    void init();
    void update();

    volatile bool enabled; // Set false in Watch to return target ownership to Watch/USART6.
    volatile bool initialized;
    volatile bool online;
    volatile uint32_t frames;
    volatile uint16_t channels[16]; // Watch channels[0] is CH1, channels[2] is CH3.
    volatile bool frameLost;
    volatile bool failsafe;
    volatile float gmDemand;
    volatile float m2006Demand;
    volatile float gmTargetDeg;
    volatile float m2006TargetDeg;
    TaskMonitor taskMonitor;
    SbusUartReceiver receiver;

private:
    StickTarget _gmTarget;
    StickTarget _mTarget;
    uint32_t _lastTargetMs;
};
}

extern garage::RemoteControlTask remoteControl;
extern "C" {
#endif
void RemoteControl_Init(void);
void RemoteControl_Update(void);
#ifdef __cplusplus
}
#endif
#endif
