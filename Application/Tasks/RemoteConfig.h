#ifndef GARAGE_REMOTE_CONFIG_H
#define GARAGE_REMOTE_CONFIG_H

#include "Libraries/Remote/StickTarget.h"

namespace garage
{
namespace remote_config
{
// Channel numbers are ONE-BASED. Verify against the transmitter's channel monitor.
// Typical Mode 2: left vertical = CH3, right horizontal = CH1.
const unsigned GM6020_CHANNEL = 3;
const unsigned M2006_CHANNEL = 1;
const uint32_t LINK_TIMEOUT_MS = 100;
const uint32_t MOTOR_FEEDBACK_TIMEOUT_MS = 50;
const uint32_t TARGET_PERIOD_MS = 20;

// low, center, high, deadband, reversed, max target deg/s, max lead angle deg.
const StickSettings GM6020_STICK = {172, 992, 1811, 40, false, 45.0f, 15.0f};
const StickSettings M2006_STICK = {172, 992, 1811, 40, false, 60.0f, 15.0f};
}
}
#endif
