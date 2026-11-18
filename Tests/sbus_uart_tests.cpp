#include "Libraries/Remote/SbusUartReceiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks=0;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static uint8_t *rx=0;
static uint32_t now=0;
static unsigned receiveCalls=0,aborts=0;
static bool failReceive=false;
static bool errorDuringReceive=false;
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *uart,uint8_t *buffer,unsigned length)
{
    ++receiveCalls; CHECK(length==1);
    if(failReceive)return HAL_BUSY;
    uart->ErrorCode=0;rx=buffer;
    if(errorDuringReceive)
    {
        // Model RX/error IRQ preemption after HAL enables reception, before return.
        errorDuringReceive=false;*rx=0;rx=0;uart->ErrorCode=4;
        garage::SbusUartReceiver::handleRxComplete(uart);
        garage::SbusUartReceiver::handleError(uart);
        uart->ErrorCode=0;
    }
    return HAL_OK;
}
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef*){++aborts;rx=0;return HAL_OK;}
uint32_t HAL_GetTick(){return now;}
static void deliver(UART_HandleTypeDef &uart,uint8_t value,uint32_t error=0)
{
    CHECK(rx!=0);*rx=value;rx=0;uart.ErrorCode=error;
    garage::SbusUartReceiver::handleRxComplete(&uart);
}
static void frame(garage::SbusUartReceiver &receiver,UART_HandleTypeDef &uart,uint8_t flags=0)
{
    uint8_t bytes[25]={0};bytes[0]=15;bytes[23]=flags;
    for(unsigned i=0;i<25;++i){if(i%8==0)++now;deliver(uart,bytes[i]);receiver.poll();}
}
int main()
{
    // An error on the first byte must survive init and trigger task-side recovery.
    {
        UART_HandleTypeDef startupUart={0};garage::SbusUartReceiver startupReceiver;
        errorDuringReceive=true;
        CHECK(!startupReceiver.init(startupUart));
        CHECK(startupReceiver.rxBytes==1 && startupReceiver.uartErrors==1);
        CHECK(startupReceiver.lastUartError==4 && startupReceiver.restarts==0);
        CHECK(!startupReceiver.ready && rx==0);
        startupReceiver.poll();
        CHECK(startupReceiver.ready && startupReceiver.restarts==1 && aborts==1);
        CHECK(receiveCalls==2 && rx!=0);
        now+=10;frame(startupReceiver,startupUart);
        CHECK(startupReceiver.decoder.usable(now));
    }
    rx=0;now=0;receiveCalls=0;aborts=0;
    UART_HandleTypeDef uart={0},other={0};garage::SbusUartReceiver receiver;
    CHECK(receiver.init(uart));frame(receiver,uart);
    CHECK(receiver.decoder.usable(now));CHECK(receiver.decoder.frame().sequence==1);
    const unsigned calls=receiveCalls;
    garage::SbusUartReceiver::handleRxComplete(&other);
    garage::SbusUartReceiver::handleError(&other);
    CHECK(receiveCalls==calls && receiver.uartErrors==0);
    // Model HAL delivering a bad byte before ErrorCallback. No rearm may clear the error.
    deliver(uart,15,1);CHECK(receiveCalls==calls && !receiver.ready);
    garage::SbusUartReceiver::handleError(&uart);CHECK(receiver.uartErrors==1);
    receiver.poll();CHECK(!receiver.decoder.usable(now) && receiver.ready && aborts==1);
    now+=10;frame(receiver,uart);CHECK(receiver.decoder.usable(now));
    for(unsigned i=0;i<128;++i)deliver(uart,0);
    CHECK(receiver.queueOverflows==1 && !receiver.ready);
    receiver.poll();CHECK(!receiver.decoder.usable(now) && receiver.ready);
    now+=10;frame(receiver,uart,8);CHECK(!receiver.decoder.usable(now));
    now+=10;frame(receiver,uart);CHECK(receiver.decoder.usable(now));
    failReceive=true;deliver(uart,15);CHECK(!receiver.ready);
    receiver.poll();CHECK(!receiver.ready && !receiver.decoder.usable(now));
    failReceive=false;receiver.poll();CHECK(receiver.ready);
    now+=10;frame(receiver,uart);CHECK(receiver.decoder.usable(now));
    now+=101;CHECK(!receiver.decoder.usable(now));
    printf("ALL UART TESTS PASSED (%u assertions)\n",checks);return 0;
}
