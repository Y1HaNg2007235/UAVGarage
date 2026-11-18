#include "UartLineReceiver.h"
#include "Libraries/Remote/SbusUartReceiver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tjc_usart_hmi.h"


namespace garage
{

UartLineReceiver *UartLineReceiver::_active = 0;

UartLineReceiver::UartLineReceiver()
    : ready(false), _uart(0), _rxByte(0), _lineLen(0), _lineReady(false)
{
}

/***********************************************************************
** 函 数 名： UartLineReceiver::init(UART_HandleTypeDef &uart)
** 函数说明： 绑定串口并启动单字节中断接收
**---------------------------------------------------------------------
** 输入参数： uart，串口句柄
** 返回参数： true 表示启动成功
***********************************************************************/
bool UartLineReceiver::init(UART_HandleTypeDef &uart)
{
    _uart = &uart;
    _lineLen = 0;
    _lineReady = false;
    _active = this;

    ready = HAL_UART_Receive_IT(_uart, &_rxByte, 1) == HAL_OK;
    return ready;
}

/***********************************************************************
** 函 数 名： UartLineReceiver::readLine(char *line, unsigned capacity)
** 函数说明： 取一条完整行，没有则返回 false
**---------------------------------------------------------------------
** 输入参数： line，输出缓冲；capacity，缓冲大小
** 返回参数： true 表示取到一行
***********************************************************************/
bool UartLineReceiver::readLine(char *line, unsigned capacity)
{
    if (!_lineReady || line == 0 || capacity == 0U)
    {
        return false;
    }

    taskENTER_CRITICAL();
    const unsigned len = _lineLen;
    for (unsigned i = 0; i < len && i + 1U < capacity; ++i)
    {
        line[i] = _line[i];
    }
    line[len < capacity ? len : capacity - 1U] = '\0';
    _lineLen = 0;
    _lineReady = false;
    taskEXIT_CRITICAL();

    return true;
}

/***********************************************************************
** 函 数 名： UartLineReceiver::onByte(uint8_t byte)
** 函数说明： 中断里累积单字节，遇换行标记一整行
**---------------------------------------------------------------------
** 输入参数： byte，收到的字节
** 返回参数： 无
***********************************************************************/
void UartLineReceiver::onByte(uint8_t byte)
{
    if (byte == '\r')
    {
        return;
    }
    if (byte == '\n')
    {
        if (_lineLen > 0U)
        {
            _lineReady = true;
        }
        return;
    }
    // 上一行还没被取走时丢弃新字节，避免覆盖
    if (!_lineReady && _lineLen < LINE_BUFFER_SIZE - 1U)
    {
        _line[_lineLen++] = static_cast<char>(byte);
    }
}

/***********************************************************************
** 函 数 名： UartLineReceiver::handleRxComplete(UART_HandleTypeDef *uart)
** 函数说明： 接收完成回调转交，并重新启动接收
**---------------------------------------------------------------------
** 输入参数： uart，串口句柄
** 返回参数： 无
***********************************************************************/
void UartLineReceiver::handleRxComplete(UART_HandleTypeDef *uart)
{
    if (_active == 0 || _active->_uart != uart)
    {
        return;
    }
    _active->onByte(_active->_rxByte);
    HAL_UART_Receive_IT(uart, &_active->_rxByte, 1);
}

}

/***********************************************************************
** 函 数 名： HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
** 函数说明： 将 HAL 接收完成回调转交给串口行接收器
**---------------------------------------------------------------------
** 输入参数： huart，串口句柄
** 返回参数： 无
***********************************************************************/
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        garage::SbusUartReceiver::handleRxComplete(huart);
    }
else if(huart->Instance == UART7)
{
    HAL_UART_Receive_IT(&huart7,(uint8_t *)RxBuffer,1);
	    ledcontrol = RxBuffer[0];     // ← 保留，存值

}
else
    {
        garage::UartLineReceiver::handleRxComplete(huart);
    }

}
