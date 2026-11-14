#ifndef GARAGE_PID_CONTROLLER_H
#define GARAGE_PID_CONTROLLER_H

namespace garage
{

// 带输出限幅和积分限幅的位置式 PID 控制器。
class PidController
{
public:
    PidController();

    void setGains(float kp, float ki, float kd);
    void setOutputLimit(float limit);   // 输出对称限幅
    void setIntegralLimit(float limit); // 积分项对称限幅
    void reset();

    // 单次更新，dt 单位秒，返回限幅后的输出。
    float update(float error, float dt);

    float output; // 最近一次输出，方便观察

private:
    float _kp;
    float _ki;
    float _kd;
    float _outputLimit;
    float _integralLimit;
    float _integral;
    float _lastError;
    bool  _first;
};

}

#endif
