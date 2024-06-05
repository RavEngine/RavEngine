// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>
#include <vector>

enum class PixEventType
{
    Begin = 0,
    End = 1,
    Marker = 2,
    BeginUtf8 = 3,
    MarkerUtf8 = 4
};

#pragma pack(push, 1)
struct PixCpuEvent
{
    INT64 Timestamp = 0;
    union
    {
        PCWSTR Name = nullptr;
        PCSTR NameUtf8;
    };
    PCWSTR FormatString = nullptr; // TODO MSFT:20105268 - Currently unused
    UINT32 Color = 0;
    PixEventType Type = PixEventType::Begin;
    BOOL HasContext = FALSE;
};
#pragma pack(pop)

#pragma pack(1)
struct DecodedPixEventBlock
{
    UINT32 ProcessId = 0;
    UINT32 ThreadId = 0;
    std::vector<PixCpuEvent> Events;
    std::vector<uint64_t> D3D12Contexts; // command list, command queue, or nothing (contextless event)
    std::vector<std::wstring> Names;
};
#pragma pack()

struct DecodedNameAndColor
{
    std::string Name;
    UINT32 Color;
};
