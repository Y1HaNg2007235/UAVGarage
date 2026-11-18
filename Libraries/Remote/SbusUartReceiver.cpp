#include "SbusUartReceiver.h"
#include "FreeRTOS.h"
#include "task.h"

namespace garage
{

SbusUartReceiver *SbusUartReceiver::_active = 0;

SbusUartReceiver::SbusUartReceiver()
    : ready(false), rxBytes(0), uartErrors(0), lastUartError(0), queueOverflows(0),
      restarts(0), _uart(0), _rxByte(0), _read(0), _write(0), _restart(false)
{
}

bool SbusUartReceiver::init(UART_HandleTypeDef &uart)
{
    _uart = &uart;
    _active = this;
    _read = _write = 0;
    decoder.discardPartial();
    // RX/error interrupts may run inside HAL_UART_Receive_IT before it returns.
    // Set the initial flags first so an ISR's recovery request is not overwritten.
    ready = true;
    _restart = false;
    if (HAL_UART_Receive_IT(_uart, &_rxByte, 1) != HAL_OK)
    {
        ready = false;
        _restart = true;
    }
    return ready;
}

void SbusUartReceiver::handleRxComplete(UART_HandleTypeDef *uart)
{
    if (_active == 0 || uart != _active->_uart) { return; }
    SbusUartReceiver &self = *_active;
    ++self.rxBytes;
    // HAL can deliver the received byte before its error callback. Do not rearm
    // here and clear ErrorCode, or a parity/framing error could be hidden.
    if (uart->ErrorCode != HAL_UART_ERROR_NONE) { handleError(uart); return; }
    if (self._restart) { return; }
    const unsigned next = (self._write + 1U) % QUEUE_SIZE;
    if (next == self._read)
    {
        ++self.queueOverflows;
        self._restart = true;
        self.ready = false;
        return;
    }
    self._queue[self._write].byte = self._rxByte;
    self._queue[self._write].receivedMs = HAL_GetTick();
    __DMB();
    self._write = next;
    if (HAL_UART_Receive_IT(uart, &self._rxByte, 1) != HAL_OK)
    {
        self.ready = false;
        self._restart = true;
    }
}

void SbusUartReceiver::handleError(UART_HandleTypeDef *uart)
{
    if (_active == 0 || uart != _active->_uart) { return; }
    if (!_active->_restart) { ++_active->uartErrors; }
    _active->lastUartError = uart->ErrorCode;
    _active->ready = false;
    _active->_restart = true;
}

void SbusUartReceiver::poll()
{
    if (_uart == 0) { return; }
    if (_restart)
    {
        // UART5 has no DMA. AbortReceive only clears UART state/interrupts here.
        taskENTER_CRITICAL();
        HAL_UART_AbortReceive(_uart);
        _read = _write = 0;
        _restart = false;
        decoder.discardPartial();
        ready = HAL_UART_Receive_IT(_uart, &_rxByte, 1) == HAL_OK;
        _restart = !ready;
        ++restarts;
        taskEXIT_CRITICAL();
        return;
    }
    for (unsigned count = 0; count < QUEUE_SIZE; ++count)
    {
        Sample sample;
        bool haveByte = false;
        taskENTER_CRITICAL();
        if (!_restart && _read != _write)
        {
            sample = _queue[_read];
            _read = (_read + 1U) % QUEUE_SIZE;
            haveByte = true;
        }
        taskEXIT_CRITICAL();
        if (!haveByte) { break; }
        decoder.push(sample.byte, sample.receivedMs);
    }
    if (_restart) { decoder.discardPartial(); }
}

}

// No original UART error callback exists. Other UARTs retain the old no-op behavior.
extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    garage::SbusUartReceiver::handleError(uart);
}
