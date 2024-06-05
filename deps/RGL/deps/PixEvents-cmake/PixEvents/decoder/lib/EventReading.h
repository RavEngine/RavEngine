// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace PixEventDecoder
{
    struct EventData
    {
        UINT64 Time; //timestamp decoded from the event
        PixOp Op; //event operation code
        UINT64 Metadata; //user supplied metadata if present for specific operation code, 0 otherwise
        UINT64 Context; // if this is a graphics context operation then this value is filled out
        UINT32 Length; //length of the resulting string
        UINT32 FormatStringBytesUsed; //total number of bytes used by the format string
        UINT32 TotalBytesUsed; //total number of bytes used by the event

        EventData()
            : Time(0ull)
            , Op(PixOp_Invalid)
            , Metadata(0ull)
            , Context(0ull)
            , Length(0)
            , FormatStringBytesUsed(0)
            , TotalBytesUsed(0)
        {}
    };

    EventData ReadEndContextEvent(
        _In_reads_to_ptr_(limit) const UINT64* source
    );

    EventData ReadEventWithNoFormatParameters(
        UINT64 eventInfo,
        _In_reads_to_ptr_(limit) const UINT64* source,
        _In_ const UINT64* limit,
        _Out_writes_(bufferLength) wchar_t* buffer,
        UINT32 bufferLength);

    EventData ReadEventWithFormatParameters(
        UINT64 eventInfo,
        _In_reads_to_ptr_(limit) const UINT64* source,
        _In_ const UINT64* limit,
        _Out_writes_(bufferLength) wchar_t* unicodeBuffer, //contains the resulting formatted string
        _Out_writes_(bufferLength) char* ansiBuffer, //only used when event is an ANSI event for conversion to UNICODE
        UINT32 bufferLength,
        _Out_writes_opt_(PIX_MAX_ARGUMENTS) UINT64* arguments = nullptr,
        _Out_opt_ UINT32* pArgumentsCount = nullptr);
}
