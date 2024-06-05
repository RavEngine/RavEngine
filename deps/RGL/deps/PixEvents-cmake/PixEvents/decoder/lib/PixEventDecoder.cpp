// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <PixEventDecoder.h>

#include "EventReading.h"
#include "BlockParser.h"

namespace PixEventDecoder
{
    DecodedPixEventBlock DecodeTimingBlock(bool ignoreEventContexts, uint32_t bufferSize, uint8_t* buffer, ConvertClockToNanoseconds const& convertClockToNanoseconds)
    {
        DecodedPixEventBlock decodedData;

        // Predict max number of events possible based on buffer size and smallest PIX event possible
        uint32_t maxEventsInBuffer = bufferSize / sizeof(uint64_t);

        // Pre-allocate buffers to avoid re-allocations below
        decodedData.Events.reserve(maxEventsInBuffer);
        decodedData.Names.reserve(maxEventsInBuffer);
        decodedData.D3D12Contexts.reserve(maxEventsInBuffer);

        if (!buffer || !convertClockToNanoseconds)
            return decodedData;

        bool isFirstEventInBlock = true;

        auto parser = std::make_unique<BlockParser>(reinterpret_cast<PEvtBlkHdr const*>(buffer), bufferSize, convertClockToNanoseconds);
        parser->ProcessEvents([&](const TimingMarkerEvent& timingEvt, PCWSTR name)
        {
            if (isFirstEventInBlock)
            {
                decodedData.ProcessId = timingEvt.cpuEvent.processId;
                decodedData.ThreadId = timingEvt.cpuEvent.threadId;
                isFirstEventInBlock = false;
            }

            decodedData.Events.push_back({
                (INT64)timingEvt.cpuEvent.timestamp,
                nullptr, // null name until all events are added
                nullptr, // null FormatString for now (MSFT:20105268)
                timingEvt.cpuEvent.color,
                timingEvt.cpuEvent.type,
                ignoreEventContexts ? FALSE : timingEvt.bContextEvent, // HasContext
                });

            decodedData.Names.push_back(name ? name : L"");
            decodedData.D3D12Contexts.push_back(timingEvt.bContextEvent ? timingEvt.pObject : 0);
        });

        // Re-assign event names now that decodedNameBuffer is done being built
        for (size_t i = 0; i < decodedData.Events.size(); i++)
        {
            decodedData.Events[i].Name = decodedData.Names[i].c_str();
        }

        return decodedData;
    }

    std::optional<DecodedNameAndColor> TryDecodePIXBeginEventOrPIXSetMarkerBlob(const UINT64* source, const UINT64* limit)
    {
        DecodedNameAndColor output;

        EventData eventData;
        static const UINT32 bufferLength = 16 * 1024;
        wchar_t unicodeBuffer[bufferLength];
        char ansiBuffer[bufferLength];
        UINT64 eventInfo = *source++;

        UINT64 timestamp = 0;
        PixOp opcode = PixOp_Invalid;
        UINT8 eventSize = 0;
        UINT8 eventMetadata = 0;
        PixOp legacyOpcode = PixOp_Invalid;
        PIXDecodeEventInfo(eventInfo, &timestamp, &opcode, &eventSize, &eventMetadata, &legacyOpcode);

        if (legacyOpcode == PixOp_BeginEvent_NoVarArgs ||
            legacyOpcode == PixOp_BeginEvent_OnContext_NoVarArgs ||
            legacyOpcode == PixOp_SetMarker_NoVarArgs ||
            legacyOpcode == PixOp_SetMarker_OnContext_NoVarArgs)
        {
            eventData = ReadEventWithNoFormatParameters(eventInfo, source, limit, unicodeBuffer, bufferLength);
        }
        else if (opcode == PixOp_BeginEvent || opcode == PixOp_SetMarker)
        {
            eventData = ReadEventWithFormatParameters(eventInfo, source, limit, unicodeBuffer, ansiBuffer, bufferLength);
        }
        else
        {
            return std::nullopt;
        }

        // Convert to UTF-8 (reusing the ANSI buffer to receive the UTF-8 data)
        int len = WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, -1, ansiBuffer, sizeof(ansiBuffer), NULL, NULL);
        if (len > 0)
        {
            output.Name = std::string(ansiBuffer, ansiBuffer + len - 1);
        }
        else
        {
            // Use empty string for failed conversions
            output.Name = std::string();
        }

        output.Color = eventData.Metadata;

        return output;
    }
}
