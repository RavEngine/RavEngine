// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>
#include <optional>

#include "DecodedPixEventTypes.h"

namespace PixEventDecoder
{
    using ConvertClockToNanoseconds = std::function<uint64_t(uint64_t)>;

    DecodedPixEventBlock DecodeTimingBlock(bool ignoreEventContexts, uint32_t bufferSize, uint8_t* buffer, ConvertClockToNanoseconds const& convertClockToNanoseconds);

    std::optional<DecodedNameAndColor> TryDecodePIXBeginEventOrPIXSetMarkerBlob(_In_reads_to_ptr_(limit) const UINT64* source, _In_ const UINT64* limit);
}
