#include "SbusDecoder.h"
#include <string.h>

namespace garage
{

SbusDecoder::SbusDecoder()
    : _used(0), _lastByteMs(0), _haveFrame(false), _transportValid(false)
{
    memset(_bytes, 0, sizeof(_bytes));
    memset(&_frame, 0, sizeof(_frame));
}

void SbusDecoder::discardPartial()
{
    _used = 0;
    _transportValid = false;
}

bool SbusDecoder::usable(uint32_t nowMs, uint32_t timeoutMs) const
{
    return _haveFrame && _transportValid && !_frame.frameLost && !_frame.failsafe &&
           static_cast<uint32_t>(nowMs - _frame.receivedMs) <= timeoutMs;
}

bool SbusDecoder::push(uint8_t byte, uint32_t receivedMs)
{
    if (_used != 0U && static_cast<uint32_t>(receivedMs - _lastByteMs) > 4U)
    {
        _used = 0;
    }
    _lastByteMs = receivedMs;
    if (_used == 0U && byte != 0x0FU)
    {
        return false;
    }
    _bytes[_used++] = byte;
    if (_used < FRAME_SIZE)
    {
        return false;
    }

    const uint8_t footer = _bytes[24];
    // S.BUS1 footer 0x00; S.BUS2 channel-frame footers 04/14/24/34.
    const bool footerValid = footer == 0U || (footer & 0xCFU) == 0x04U;
    if (!footerValid || (_bytes[23] & 0xF0U) != 0U)
    {
        _transportValid = false;
        // Keep a candidate header within the rejected frame, including the final byte.
        unsigned next = 1;
        while (next < FRAME_SIZE && _bytes[next] != 0x0FU) { ++next; }
        _used = FRAME_SIZE - next;
        if (_used != 0U) { memmove(_bytes, _bytes + next, _used); }
        return false;
    }

    for (unsigned channel = 0; channel < 16; ++channel)
    {
        uint16_t value = 0;
        for (unsigned bit = 0; bit < 11; ++bit)
        {
            const unsigned index = channel * 11U + bit;
            value |= static_cast<uint16_t>(((_bytes[1U + index / 8U] >> (index % 8U)) & 1U) << bit);
        }
        _frame.channels[channel] = value;
    }
    _frame.channel17 = (_bytes[23] & 1U) != 0;
    _frame.channel18 = (_bytes[23] & 2U) != 0;
    _frame.frameLost = (_bytes[23] & 4U) != 0;
    _frame.failsafe = (_bytes[23] & 8U) != 0;
    _frame.receivedMs = receivedMs;
    ++_frame.sequence;
    _haveFrame = true;
    _transportValid = true;
    _used = 0;
    return true;
}

}
