#ifndef GARAGE_GM6020_CAN_H
#define GARAGE_GM6020_CAN_H

#include "fdcan.h"

namespace garage
{

// GM6020 的 CAN 总线封装：把 1~4 号电机的给定打包成 0x1FF 帧周期发送，
// 并轮询接收 0x204+ID 的反馈帧，更新对应电机状态。
class Gm6020Can
{
public:
    struct Feedback
    {
        uint16_t angle;         // 机械角度 0~8191
        int16_t  speedRpm;      // 转速 rpm
        int16_t  torqueCurrent; // 实际转矩电流
        int8_t   temperature;   // 温度 ℃
        uint32_t receivedMs;    // 最近一次收到反馈的时刻
        bool     valid;         // 是否收到过反馈
    };

    enum
    {
        CONTROL_ID       = 0x1FF, // 发给 ID1~4 电机的控制帧
        FEEDBACK_ID_BASE = 0x204, // 反馈帧 = 0x204 + 电机ID
        MOTOR_COUNT      = 4
    };

    Gm6020Can();
    bool init(FDCAN_HandleTypeDef &hfdcan);
    void update(); // 接收反馈 + 按 1kHz 节流发送控制帧

    void setMotorOutput(uint8_t motorId, int16_t output); // 设置 1~4 号电机给定
    bool readFeedback(uint8_t motorId, Feedback &out) const;

    bool ready;

private:
    void receive();  // 把 RxFifo0 里的反馈全部读出
    void transmit(); // 打包发送 0x1FF

    FDCAN_HandleTypeDef *_hfdcan;
    int16_t  _output[MOTOR_COUNT];
    Feedback _feedback[8]; // 索引 = 电机ID (1~7)，[0] 不用
    uint32_t _lastSendMs;
};

}

#endif
