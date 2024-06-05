// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace PixEventDecoder
{
    struct TimingCpuEvent
    {
        UINT64 timestamp;
        UINT32 name;
        PixEventType type;
        UINT32 context;
        UINT32 processId;
        UINT32 threadId;
        UINT32 color;
        UINT32 metadata;
    };

    struct TimingMarkerEvent
    {
        bool bContextEvent;
        UINT64 pObject;
        TimingCpuEvent cpuEvent;
    };

    using PixEventCallback = std::function<void(const TimingMarkerEvent&, PCWSTR)>;
    using ConvertClockToNanoseconds = std::function<uint64_t(uint64_t)>;

    class BlockParser
    {
    public:
        BlockParser(const PEvtBlkHdr* blockHeader, UINT32 blockSize, ConvertClockToNanoseconds const& convertClockToNanoseconds);

        void ProcessEvents(PixEventCallback callback);

    private:
        UINT64 const m_blockStartTime;
        UINT64 const m_blockEndTime;
        UINT64 const* const m_blockDataStart;
        UINT64 const* const m_blockDataEnd;
        UINT32 const m_processId;
        UINT32 const m_threadId;

        ConvertClockToNanoseconds m_convertClockToNanoseconds;

        static const UINT32 m_bufferLength = 16 * 1024;
        std::vector<wchar_t> m_unicodeBuffer;
        std::vector<char> m_ansiBuffer;
    };
}
