#ifndef GARAGE_M2006_H
#define GARAGE_M2006_H

#include <stdint.h>

namespace garage
{

// M2006 P36 减速电机：C610 电调反馈的是转子单圈角度（0~8191），
// 输出轴角度需按 36:1 减速比做多圈累计后再换算。
class M2006
{
public:
    enum
    {
        ENCODER_RESOLUTION = 8192,                                  // 转子单圈计数
        GEAR_RATIO = 36,                                            // 减速比
        OUTPUT_RESOLUTION = ENCODER_RESOLUTION * GEAR_RATIO         // 输出轴一圈计数
    };

    explicit M2006(uint8_t motorId);

    void setOutput(int16_t output);
    void updateFeedback(uint16_t rotorAngle, int16_t speedRpm,
                        int16_t torque, bool valid);

    uint8_t motorId;  // 电调 ID，1~4（控制帧 0x200 对应 ID 1~4）
    int16_t output;   // 最近一次设置的电流给定

    uint16_t angleRaw;      // 转子单圈角度 0~8191
    int16_t  speedRpm;      // 转子转速 rpm
    int16_t  torqueCurrent; // 实际输出转矩
    bool     feedbackValid; // 是否收到过反馈
    uint32_t feedbackMs;    // 最近一次收到反馈的时刻
    float    angleDegValue; // 输出轴角度（相对零位，0~360），Watch 看

private:
    int64_t  _rotorAccum;   // 转子累计计数（相对零位）
    uint16_t _lastRotorRaw; // 上次转子角度，回绕检测
    bool     _first;        // 首次反馈标志
};

}

#endif
