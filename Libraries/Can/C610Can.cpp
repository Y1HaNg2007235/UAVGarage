#include "C610Can.h"

namespace garage
{
namespace
{
const uint32_t SEND_PERIOD_MS = 1U; // 控制帧 1kHz 发送
}

C610Can::C610Can()
    : ready(false), _hfdcan(0), _lastSendMs(0)
{
    for (unsigned i = 0; i < MOTOR_COUNT; ++i)
    {
        _output[i] = 0;
    }
    for (unsigned i = 0; i < 9; ++i)
    {
        _feedback[i].angle = 0;
        _feedback[i].speedRpm = 0;
        _feedback[i].torqueCurrent = 0;
        _feedback[i].receivedMs = 0;
        _feedback[i].valid = false;
    }
}

/***********************************************************************
** 函 数 名： C610Can::init(FDCAN_HandleTypeDef &hfdcan)
** 函数说明： 绑定 FDCAN，配置反馈帧滤波器并启动总线
**---------------------------------------------------------------------
** 输入参数： hfdcan，CubeMX 初始化后的 FDCAN 句柄（CAN2）
** 返回参数： true 表示启动成功
***********************************************************************/
bool C610Can::init(FDCAN_HandleTypeDef &hfdcan)
{
    _hfdcan = &hfdcan;

    // 掩码模式接收 0x200~0x20F（覆盖 0x201~0x208 反馈帧，低 4 位不参与匹配）
    FDCAN_FilterTypeDef filter = {0};
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = FEEDBACK_ID_BASE;
    filter.FilterID2 = 0x7F0;
    if (HAL_FDCAN_ConfigFilter(_hfdcan, &filter) != HAL_OK)
    {
        return false;
    }

    ready = HAL_FDCAN_Start(_hfdcan) == HAL_OK;
    return ready;
}

/***********************************************************************
** 函 数 名： C610Can::setMotorOutput(uint8_t motorId, int16_t output)
** 函数说明： 记录某电调的电流给定，update 时统一发送
**---------------------------------------------------------------------
** 输入参数： motorId，电调 ID（1~4）；output，电流给定 -10000~10000
** 返回参数： 无
***********************************************************************/
void C610Can::setMotorOutput(uint8_t motorId, int16_t output)
{
    if (motorId >= 1U && motorId <= MOTOR_COUNT)
    {
        _output[motorId - 1U] = output;
    }
}

/***********************************************************************
** 函 数 名： C610Can::readFeedback(uint8_t motorId, Feedback &out) const
** 函数说明： 复制某电调的反馈快照
**---------------------------------------------------------------------
** 输入参数： motorId，电调 ID；out，接收反馈的变量
** 返回参数： true 表示反馈有效
***********************************************************************/
bool C610Can::readFeedback(uint8_t motorId, Feedback &out) const
{
    if (motorId == 0U || motorId >= 9U)
    {
        return false;
    }
    out = _feedback[motorId];
    return out.valid;
}

/***********************************************************************
** 函 数 名： C610Can::update()
** 函数说明： 轮询接收反馈，并按时发送控制帧
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void C610Can::update()
{
    if (!ready)
    {
        return;
    }
    receive();
    transmit();
}

/***********************************************************************
** 函 数 名： C610Can::receive()
** 函数说明： 读出 RxFifo0 中的全部反馈帧并解析
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void C610Can::receive()
{
    while (HAL_FDCAN_GetRxFifoFillLevel(_hfdcan, FDCAN_RX_FIFO0) > 0U)
    {
        FDCAN_RxHeaderTypeDef header;
        uint8_t data[8];
        if (HAL_FDCAN_GetRxMessage(_hfdcan, FDCAN_RX_FIFO0, &header, data) != HAL_OK)
        {
            break;
        }

        const uint32_t id = header.Identifier;
        if (id <= FEEDBACK_ID_BASE || id > FEEDBACK_ID_BASE + 8U)
        {
            continue;
        }
        const uint8_t motorId = static_cast<uint8_t>(id - FEEDBACK_ID_BASE);
        Feedback &fb = _feedback[motorId];
        fb.angle = (static_cast<uint16_t>(data[0]) << 8) | data[1];
        fb.speedRpm = static_cast<int16_t>((static_cast<uint16_t>(data[2]) << 8) | data[3]);
        fb.torqueCurrent = static_cast<int16_t>((static_cast<uint16_t>(data[4]) << 8) | data[5]);
        fb.receivedMs = HAL_GetTick();
        fb.valid = true;
    }
}

/***********************************************************************
** 函 数 名： C610Can::transmit()
** 函数说明： 把 4 路电调电流给定打包成 0x200 帧发送
**---------------------------------------------------------------------
** 输入参数： 无
** 返回参数： 无
***********************************************************************/
void C610Can::transmit()
{
    const uint32_t now = HAL_GetTick();
    if (now - _lastSendMs < SEND_PERIOD_MS)
    {
        return;
    }
    _lastSendMs = now;

    // C610 控制帧大端序：高字节在前
    uint8_t data[8];
    for (unsigned i = 0; i < MOTOR_COUNT; ++i)
    {
        const int16_t value = _output[i];
        data[i * 2U] = static_cast<uint8_t>((static_cast<uint16_t>(value) >> 8) & 0xFFU);
        data[i * 2U + 1U] = static_cast<uint8_t>(static_cast<uint16_t>(value) & 0xFFU);
    }

    FDCAN_TxHeaderTypeDef header = {0};
    header.Identifier = CONTROL_ID;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = FDCAN_DLC_BYTES_8;
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    header.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(_hfdcan, &header, data);
}

}
