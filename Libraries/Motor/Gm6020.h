#ifndef GARAGE_GM6020_H
#define GARAGE_GM6020_H

#include <stdint.h>

namespace garage
{

// GM6020 直流无刷电机：保存单电机反馈状态，并输出电流/电压给定。
// 机械角度 0~8191 对应输出轴 0~360°。
class Gm6020
{
public:
    enum { ENCODER_RESOLUTION = 8192 };

    explicit Gm6020(uint8_t motorId);

    void setOutput(int16_t output);
    void updateFeedback(uint16_t angle, int16_t speedRpm,
                        int16_t torqueCurrent, int8_t temperature, bool valid);

    uint8_t motorId; // 拨码开关 ID，1~4（反馈帧 0x204+ID）
    int16_t output;  // 最近一次设置的给定值

    uint16_t angleRaw;      // 机械角度 0~8191
    int16_t  speedRpm;      // 转速 rpm
    int16_t  torqueCurrent; // 实际转矩电流
    int8_t   temperature;   // 温度 ℃
    bool     feedbackValid; // 是否收到过反馈
    uint32_t feedbackMs;    // 最近一次收到反馈的时刻
    float    angleDegValue; // 当前角度（度 0~360），Watch 里直接看

    float angleDeg() const;

    static float countToDeg(uint16_t count);
};

}

#endif
