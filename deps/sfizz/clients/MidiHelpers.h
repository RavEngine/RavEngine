#pragma once
#include <cstdint>

namespace midi {
constexpr uint8_t statusMask { 0b11110000 };
constexpr uint8_t channelMask { 0b00001111 };
constexpr uint8_t noteOff { 0x80 };
constexpr uint8_t noteOn { 0x90 };
constexpr uint8_t polyphonicPressure { 0xA0 };
constexpr uint8_t controlChange { 0xB0 };
constexpr uint8_t programChange { 0xC0 };
constexpr uint8_t channelPressure { 0xD0 };
constexpr uint8_t pitchBend { 0xE0 };
constexpr uint8_t systemMessage { 0xF0 };

constexpr uint8_t status(uint8_t midiStatusByte)
{
    return midiStatusByte & statusMask;
}
constexpr uint8_t channel(uint8_t midiStatusByte)
{
    return midiStatusByte & channelMask;
}

constexpr int buildAndCenterPitch(uint8_t firstByte, uint8_t secondByte)
{
    return (int)(((unsigned int)secondByte << 7) + (unsigned int)firstByte) - 8192;
}
}
