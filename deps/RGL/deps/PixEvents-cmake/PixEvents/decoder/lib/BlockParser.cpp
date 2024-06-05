// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "BlockParser.h"

#include "EventReading.h"

namespace PixEventDecoder
{
    int ParseFormatArgument(_In_ PCWSTR pString)
    {
        assert(pString[0] == L'%');

        for (int index = 2;; index++)
        {
            WCHAR c = pString[index - 1];
            switch (c)
            {
            case 0:
                // The string ended prematurely.  Back up one so that the result
                // still points to the end-of-string character:

                index--;
                return -index;

            case L'%':
                return -index;

            case L'c':
            case L'C':
            case L'd':
            case L'i':
            case L'o':
            case L'u':
            case L'x':
            case L'X':
            case L'e':
            case L'E':
            case L'f':
            case L'g':
            case L'G':
            case L'a':
            case L'A':
            case L'p':
            case L's':
            case L'S':
            case L'n':
                return index;
            }
        }
    }

    // Returns the number of QWORDS occupied in the stream by the mangled string.
    // Always a multiple of 8 because the string is padded/written as UINT64's.
    int UnmangleString(_In_reads_to_ptr_(pInStop) BYTE *pIn, _Out_writes_z_(PIX_MAX_EVENT_CHARACTERS + 1) WCHAR *pOut, _In_opt_ BYTE* pInStop = nullptr)
    {
        int intIndex = 0;

        for (;;)
        {
            WCHAR *pByteQuad = (WCHAR*)(pIn + (intIndex * sizeof(UINT64)));

            // Don't read beyond the specified read point
            if ((pInStop != nullptr) && ((BYTE*)pByteQuad >= pInStop))
            {
                *pOut = 0;
                intIndex--;
                break;
            }

            if (((*pOut++) = pByteQuad[3]) == 0) break;
            if (((*pOut++) = pByteQuad[2]) == 0) break;
            if (((*pOut++) = pByteQuad[1]) == 0) break;
            if (((*pOut++) = pByteQuad[0]) == 0) break;
            if ((intIndex + 1) == (PIX_MAX_EVENT_CHARACTERS * sizeof(WCHAR) / sizeof(UINT64)))
            {
                *pOut = 0;
                break;
            }

            intIndex += 1;
        }

        return intIndex + 1;
    }

    bool IsKnownOpcode(PixOp opcode)
    {
        switch (opcode)
        {
        case PixOp_EndEvent: __fallthrough;
        case PixOp_BeginEvent: __fallthrough;
        case PixOp_SetMarker: __fallthrough;
            return true;
        default:
            return false;
        }
    }

    bool IsValidEventInfo(UINT64 maxTime, UINT64 previousTime, UINT64 maskedTimeBits, UINT64 eventInfo)
    {
        UINT64 time = 0;
        PixOp opcode = PixOp_Invalid;
        UINT8 eventSize = 0;
        UINT8 eventMetadata = 0;
        PixOp legacyOpcode = PixOp_Invalid;
        //time and opcode are decoded twice, context uses the same bits as alignment
        PIXDecodeEventInfo(eventInfo, /*out*/ &time, /*out*/ &opcode, /*out*/ &eventSize, /*out*/ &eventMetadata, &legacyOpcode);

        if (!IsKnownOpcode(opcode))
        {
            return false;
        }

        if (eventSize > 0)
        {
            // There are no reserved bits in V2
        }
        else
        {
            if ((eventInfo & PIX_EVENT_RESERVED_READ_BITMASK) != 0 ||
                (eventInfo & PIX_EVENT_PHASE_READ_BITMASK) != 0 ||
                (eventInfo & PIX_EVENT_HAS_GPU_READ_BITMASK) != 0)
            {
                return false;
            }
        }

        time |= maskedTimeBits;
        //time must increase
        //this check is not friendly to overflow of lower part of timestamp
        if (time < previousTime)
        {
            return false;
        }

        if (time > maxTime)
        {
            return false;
        }

        return true;
    }

    BlockParser::BlockParser(const PEvtBlkHdr* blockHeader, UINT32 blockSize, ConvertClockToNanoseconds const& convertClockToNanoseconds) :
        m_blockStartTime(blockHeader->cpuHeader.beginTimestamp),
        m_blockEndTime(blockHeader->cpuHeader.endTimestamp),
        m_blockDataStart(reinterpret_cast<const UINT64*>(reinterpret_cast<const BYTE*>(blockHeader) + sizeof(PEvtBlkHdr))),
        m_blockDataEnd(reinterpret_cast<const UINT64*>(reinterpret_cast<const BYTE*>(blockHeader) + blockSize)),
        m_processId(blockHeader->cpuHeader.processId),
        m_threadId(blockHeader->cpuHeader.threadId),
        m_convertClockToNanoseconds(convertClockToNanoseconds)
    {
        assert(blockHeader->BlockType == PIXEVT_CPU_BLOCK);
        assert(blockSize > 0);

        m_unicodeBuffer.resize(m_bufferLength);
        m_ansiBuffer.resize(m_bufferLength);
    }

    void BlockParser::ProcessEvents(PixEventCallback callback)
    {
        assert(callback != nullptr);
        assert(m_blockDataStart != nullptr);
        assert(m_blockDataEnd != nullptr);
        static const UINT8 c_eventSizeMax = PIXEventsSizeMax;

        //TODO: exclude blocks outside the range if the range was specified

        const UINT64* currentPosition = m_blockDataStart;
        UINT64 maskedTimeBits = m_blockStartTime & ~PIXEventsTimestampWriteMask;
        UINT64 previousTimestamp = m_blockStartTime;

        while (currentPosition < m_blockDataEnd && *currentPosition != PIXEventsBlockEndMarker)
        {
            TimingCpuEvent currentEvent = {};
            UINT64 eventInfo = *currentPosition++;
            UINT64 time = 0;
            PixOp opcode = PixOp_Invalid;
            PCWSTR eventName = nullptr;
            UINT8 eventSize = 0;
            UINT8 eventMetadata = 0;
            PixOp legacyOpcode = PixOp_Invalid;
            PIXDecodeEventInfo(eventInfo, &time, &opcode, &eventSize, &eventMetadata, &legacyOpcode);

            if (!IsKnownOpcode(opcode))
            {
                if (eventSize > 0)
                {
                    currentPosition += eventSize - 1;
                }

                continue;
            }

            //restore timestamp to full 64-bit format
            //only bottom 44 bits of the timestamp are written in an event
            //top 20 bits are added from the block start timestamp
            //it could happen that bottom 44 bit part overflows
            //in this case we use the fact that all timestamps in the blocks are written in non-descending order
            //we increase top 20 bits value by (mask + 1) which is an equivalent of adding carry flag
            //we add (mask + 1) to the restored timestamp so it is greater or equal than the previous timestamp
            time |= maskedTimeBits;
            if (time < previousTimestamp)
            {
                maskedTimeBits += (PIXEventsTimestampWriteMask + 1);
                time += (PIXEventsTimestampWriteMask + 1);
            }

            previousTimestamp = time;
            if (m_convertClockToNanoseconds)
            {
                currentEvent.timestamp = m_convertClockToNanoseconds(time);
            }

            currentEvent.context = 0;
            currentEvent.processId = m_processId;
            currentEvent.threadId = m_threadId;
            const bool bIsContextEvent = (eventMetadata & PIX_EVENT_METADATA_ON_CONTEXT) != 0;
            EventData eventData = {};
            switch (opcode)
            {

            case PixOp_EndEvent:
            {
                currentEvent.type = PixEventType::End;

                if (bIsContextEvent)
                {
                    eventData = ReadEndContextEvent(currentPosition);
                    if (eventSize > 0 && eventSize < c_eventSizeMax)
                    {
                        currentPosition += eventSize - 1;
                    }
                    else
                    {
                        currentPosition += eventData.TotalBytesUsed / sizeof(UINT64);
                    }
                }
            }
            break;

            case PixOp_BeginEvent: __fallthrough;
            case PixOp_SetMarker: __fallthrough;
            {
                // All V2 events are marked as VarArgs.
                // The reader figures out if it should use the non VarArgs fast path
                if (legacyOpcode == PixOp_BeginEvent_NoVarArgs ||
                    legacyOpcode == PixOp_SetMarker_NoVarArgs ||
                    legacyOpcode == PixOp_BeginEvent_OnContext_NoVarArgs ||
                    legacyOpcode == PixOp_SetMarker_OnContext_NoVarArgs)
                {
                    eventData = ReadEventWithNoFormatParameters(eventInfo, currentPosition, m_blockDataEnd, m_unicodeBuffer.data(), m_bufferLength);
                    if (eventSize > 0 && eventSize < c_eventSizeMax)
                    {
                        currentPosition += eventSize - 1;
                    }
                    else
                    {
                        currentPosition += eventData.TotalBytesUsed / sizeof(UINT64);
                    }
                    eventName = m_unicodeBuffer.data();
                    currentEvent.type = (opcode == PixOp_BeginEvent)
                        ? PixEventType::Begin
                        : PixEventType::Marker;
                }
                else
                {
                    eventData = ReadEventWithFormatParameters(eventInfo, currentPosition, m_blockDataEnd, m_unicodeBuffer.data(), m_ansiBuffer.data(), m_bufferLength);

                    if (eventSize > 0 && eventSize < c_eventSizeMax)
                    {
                        currentPosition += eventSize - 1;
                    }
                    else
                    {
                        //
                        // Unfortunately, the event buffer format doesn't indicate how
                        // many parameters are expected (or how many bytes to skip to
                        // account for the parameters). This can cause problems when the
                        // format string doesn't contain any format specifiers. For
                        // example: PIXSetMarker("Foo", 123).
                        //
                        // The peek loops below try and account for this by looking for
                        // the next thing that looks like a valid event. This is fine,
                        // unless we have some data in there that looks like a valid
                        // event. As most bit patterns are valid events (eg "0" is a
                        // valid opcode) we end up having to rely on the timestamp. We
                        // can reject any events that are before the one we just
                        // processed ("time") and any events that are after the last
                        // event in the block ("m_blockEndTime").
                        //
                        // Ideally we'd modify the emitting code to be more robust, but
                        // we have the added complication that we need to support old
                        // code generating these events (including Windows OS
                        // components).
                        //

                        //peek at the event after the current event
                        //if reached the end of the data block, then look for extra events after the current format string
                        const UINT64* peekPosition = currentPosition + eventData.TotalBytesUsed / sizeof(UINT64);
                        if ((peekPosition < m_blockDataEnd && *peekPosition != PIXEventsBlockEndMarker && !IsValidEventInfo(m_blockEndTime, time, maskedTimeBits, *peekPosition))
                            || (peekPosition >= m_blockDataEnd))
                        {
                            //does not look like the correct event info or event was truncated
                            //start looking for the next event right after the format string of the current event
                            peekPosition = currentPosition + 1 + eventData.FormatStringBytesUsed / sizeof(UINT64); //metadata + format string
                            while (peekPosition < m_blockDataEnd && *peekPosition != PIXEventsBlockEndMarker && !IsValidEventInfo(m_blockEndTime, time, maskedTimeBits, *peekPosition))
                            {
                                ++peekPosition;
                            }
                        }
                        currentPosition = peekPosition;
                    }

                    eventName = m_unicodeBuffer.data();
                    currentEvent.type = (opcode == PixOp_BeginEvent)
                        ? PixEventType::Begin
                        : PixEventType::Marker;
                }
            }
            break;

            default:
                assert(false && "How did we reach this point? IsKnownOpcode() above should have filtered this out");
                break;
            }

            assert(eventData.TotalBytesUsed % sizeof(UINT64) == 0);

            currentEvent.color = static_cast<UINT32>(eventData.Metadata);
            callback({ bIsContextEvent, eventData.Context, currentEvent }, eventName);
        }
    }
}
