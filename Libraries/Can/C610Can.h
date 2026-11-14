#ifndef GARAGE_C610_CAN_H
#define GARAGE_C610_CAN_H

#include "fdcan.h"

namespace garage
{

// C610 无刷电调 CAN 总线封装：把 1~4 号电调的电流给定打包成 0x200 帧发送，
// 轮询接收 0x200+ID 的反馈帧（电调 ID 1~8 对应反馈 0x201~0x208）。
class C610Can
{
public:
    struct Feedback
    {
        uint16_t angle;        // 转子机械角度 0~8191
        int16_t  speedRpm;     // 转子转速 rpm
        int16_t  torqueCurrent; // 实际输出转矩
        uint32_t receivedMs;    // 最近一次收到反馈的时刻
        bool     valid;         // 是否收到过反馈
    };

    enum
    {
        CONTROL_ID = 0x200,       // 发给 ID1~4 电调的控制帧
        FEEDBACK_ID_BASE = 0x200, // 反馈帧 = 0x200 + 电调ID
        MOTOR_COUNT = 4
    };

    C610Can();
    bool init(FDCAN_HandleTypeDef &hfdcan);
    void update(); // 接收反馈 + 按 1kHz 节流发送控制帧

    void setMotorOutput(uint8_t motorId, int16_t output); // 设置 1~4 号电调电流给定
    bool readFeedback(uint8_t motorId, Feedback &out) const;

    bool ready;

private:
    void receive(); // 把 RxFifo0 里的反馈全部读出
    void transmit(); // 打包发送 0x200

    FDCAN_HandleTypeDef *_hfdcan;
    int16_t  _output[MOTOR_COUNT];
    Feedback _feedback[9]; // 索引 = 电调ID (1~8)，[0] 不用
    uint32_t _lastSendMs;
};

}

#endif
