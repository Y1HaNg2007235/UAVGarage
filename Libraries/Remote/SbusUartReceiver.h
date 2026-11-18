#ifndef GARAGE_SBUS_UART_RECEIVER_H
#define GARAGE_SBUS_UART_RECEIVER_H

#include "SbusDecoder.h"
#include "stm32h7xx_hal.h"

namespace garage
{

// One ISR producer, one rcProtocolTask consumer. No DMA memory/cache dependencies.
class SbusUartReceiver
{
public:
    SbusUartReceiver();
    bool init(UART_HandleTypeDef &uart);
    void poll();
    static void handleRxComplete(UART_HandleTypeDef *uart);
    static void handleError(UART_HandleTypeDef *uart);

    SbusDecoder decoder;
    volatile bool ready;
    volatile uint32_t rxBytes;
    volatile uint32_t uartErrors;
    volatile uint32_t lastUartError;
    volatile uint32_t queueOverflows;
    volatile uint32_t restarts;

private:
    struct Sample { uint32_t receivedMs; uint8_t byte; };
    enum { QUEUE_SIZE = 128 };
    static SbusUartReceiver *_active;
    UART_HandleTypeDef *_uart;
    uint8_t _rxByte;
    Sample _queue[QUEUE_SIZE];
    volatile unsigned _read;
    volatile unsigned _write;
    volatile bool _restart;
};

}
#endif
