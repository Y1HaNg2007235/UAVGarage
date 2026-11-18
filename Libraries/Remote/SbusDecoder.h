#ifndef GARAGE_SBUS_DECODER_H
#define GARAGE_SBUS_DECODER_H

#include <stdint.h>

namespace garage
{

// Receives bytes after the MC02's hardware inverter: UART 100000 baud, 8E2.
// STM32 HAL must use 9-bit word length with even parity for eight payload bits.
class SbusDecoder
{
public:
    struct Frame
    {
        uint16_t channels[16];
        bool channel17;
        bool channel18;
        bool frameLost;
        bool failsafe;
        uint32_t receivedMs;
        uint32_t sequence;
    };

    SbusDecoder();
    bool push(uint8_t byte, uint32_t receivedMs);
    void discardPartial(); // UART parity/framing error or byte queue overflow.
    bool usable(uint32_t nowMs, uint32_t timeoutMs = 100U) const;
    const Frame &frame() const { return _frame; }

private:
    enum { FRAME_SIZE = 25 };
    uint8_t _bytes[FRAME_SIZE];
    unsigned _used;
    uint32_t _lastByteMs;
    bool _haveFrame;
    bool _transportValid;
    Frame _frame;
};

}
#endif
