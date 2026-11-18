#ifndef GARAGE_UART_LINE_RECEIVER_H
#define GARAGE_UART_LINE_RECEIVER_H

#include "stm32h7xx_hal.h"
#include "main.h"


namespace garage
{

// 串口行接收器：单字节中断接收，按换行累积成一行，任务轮询取完整行。
class UartLineReceiver
{
public:
    enum { LINE_BUFFER_SIZE = 64 };

    UartLineReceiver();
    bool init(UART_HandleTypeDef &uart);

    // 取一条完整行（不含换行符），成功返回 true。
    bool readLine(char *line, unsigned capacity);

    bool ready;

    static void handleRxComplete(UART_HandleTypeDef *uart);

private:
    void onByte(uint8_t byte);

    UART_HandleTypeDef *_uart;
    uint8_t  _rxByte;                 // 中断接收的单字节
    char     _line[LINE_BUFFER_SIZE];
    unsigned _lineLen;
    volatile bool _lineReady;

    static UartLineReceiver *_active;
};

}

#endif
