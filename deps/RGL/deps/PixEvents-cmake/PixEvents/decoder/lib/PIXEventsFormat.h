// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

//
// Common header for sharing event format between runtime and decoding library
//
#pragma once
#include <pix3.h>
#include <PIXEventsCommon.h>

const int PIX_MAX_ARGUMENTS = 16;

// Maximum number of characters allowed in the format string given to
// PIXBeginEvent or PIXSetMarker (including the end-of-string).  We
// add 4 for alignment purposes.  The total must be a multiple of 4.

#define PIX_MAX_EVENT_CHARACTERS 36

// Also note that UmdOpCode starts with PixOP_Umd_SegmentStart)
// When adding an enum value to PixOp that is related to public PIX Events, add a mirrored value to PIXEventType
// and use the value from PIXEventType in public headers.
// Please add a static assert to make sure mirrored value in private and public headers will always match.
// When adding new values to the enum make sure to update IsUmdPixOp method right under the enum
enum PixOp
{
    // V1
    PixOp_EndEventV1 = 0x000,
    PixOp_BeginEvent_VarArgs = 0x001,
    PixOp_BeginEvent_NoVarArgs = 0x002,
    PixOp_BeginEvent_Float = 0x003, //used only for compatibility with old captures
    PixOp_BeginEvent_Double = 0x004, //used only for compatibility with old captures
    PixOp_BeginEvent_Int = 0x005, //used only for compatibility with old captures
    PixOp_BeginEvent_Int64 = 0x006, //used only for compatibility with old captures
    PixOp_SetMarker_VarArgs = 0x007,
    PixOp_SetMarker_NoVarArgs = 0x008,

    PixOp_EndEvent_OnContext = 0x010,
    PixOp_BeginEvent_OnContext_VarArgs = 0x011,
    PixOp_BeginEvent_OnContext_NoVarArgs = 0x012,
    PixOp_SetMarker_OnContext_VarArgs = 0x017,
    PixOp_SetMarker_OnContext_NoVarArgs = 0x018,

    // V2
    PixOp_EndEvent = 0x000,
    PixOp_BeginEvent = 0x001,
    PixOp_SetMarker = 0x002,
    
    PixOp_Invalid = 0x400,    // Valid PixOp values must be less than this
};

//-------------------------------------------------------------------------------------------------
// PIXEvt CPU-side event encoding/decoding
// 6666555555555544444444443333333333222222222211111111110000000000
// 3210987654321098765432109876543210987654321098765432109876543210
// TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTOOOOOOOOOOPPRGCCCCCC

// T = timestamp (CPU), 44 bits
// O = opcode, 10 bits
// R = reserved, 1 bit
// C = context, 6 bits
// G = has matching GPU timestamp, 1 bit
// P = corresponds to the phase of an event, 2 bits

// Reserved: Bit 7 (1 bit)
#define PIX_EVENT_RESERVED_READ_BITMASK   (0x0000000000000380)
#define PIX_EVENT_RESERVED_WRITE_BITMASK  (0x0000000000000007)
#define PIX_EVENT_RESERVED_SHIFT_BITS     (7)

// Has-matching-GPU-timestamp: Bit 6 (1 bit)
#define PIX_EVENT_HAS_GPU_READ_BITMASK    (0x0000000000000040)
#define PIX_EVENT_HAS_GPU_WRITE_BITMASK   (0x0000000000000001)
#define PIX_EVENT_HAS_GPU_SHIFT_BITS      (6)

// Context: Bits 0-5 (6 bits)
#define PIX_EVENT_CONTEXT_READ_BITMASK    (0x000000000000003F)
#define PIX_EVENT_CONTEXT_WRITE_BITMASK   (0x000000000000003F)
#define PIX_EVENT_CONTEXT_SHIFT_BITS      (0)

// Phase: Bits 8-9 (2 bits)
#define PIX_EVENT_PHASE_READ_BITMASK      (0x0000000000000300)
#define PIX_EVENT_PHASE_WRITE_BITMASK     (0x0000000000000003)
#define PIX_EVENT_PHASE_SHIFT_BITS        (8)

//alignment is not used since 1502, but we keep decoding it for backward compatibility with older timing captures
inline void PIXDecodeEventV1Info(UINT64 eventBits, UINT64* timestamp, PixOp* opcode)
{
    *timestamp = static_cast<UINT64>((eventBits & PixEventsLegacy::PIXEventsTimestampReadMask) >> PixEventsLegacy::PIXEventsTimestampBitShift);
    *opcode = static_cast<PixOp>((eventBits & PixEventsLegacy::PIXEventsTypeReadMask) >> PixEventsLegacy::PIXEventsTypeBitShift);
}

inline void PIXDecodeEventInfo(UINT64 eventBits, UINT64* timestamp, PixOp* opcode, UINT8* eventSize, UINT8* eventMetadata, PixOp* legacyOpcode)
{
    *eventSize = static_cast<UINT8>((eventBits & PIXEventsSizeReadMask) >> PIXEventsSizeBitShift);

    if (*eventSize > 0)
    {
        *eventMetadata = static_cast<UINT8>((eventBits & PIXEventsMetadataReadMask) >> PIXEventsMetadataBitShift);
        *timestamp = static_cast<UINT64>((eventBits & PIXEventsTimestampReadMask) >> PIXEventsTimestampBitShift);
        *opcode = static_cast<PixOp>((eventBits & PIXEventsTypeReadMask) >> PIXEventsTypeBitShift);
        *legacyOpcode = PixOp_Invalid;
    }
    else
    {
        *eventMetadata = 0;
        PIXDecodeEventV1Info(eventBits, timestamp, opcode);

        // Convert to V2
        *legacyOpcode = *opcode;
        switch (*opcode)
        {
        case PixOp_EndEvent:
            *opcode = PixOp_EndEvent;
            break;

        case PixOp_BeginEvent_VarArgs:
            *opcode = PixOp_BeginEvent;
            *eventMetadata |= PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_BeginEvent_NoVarArgs:
            *opcode = PixOp_BeginEvent;
            *eventMetadata |= PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_SetMarker_VarArgs:
            *opcode = PixOp_SetMarker;
            *eventMetadata |= PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_SetMarker_NoVarArgs:
            *opcode = PixOp_SetMarker;
            *eventMetadata |= PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_EndEvent_OnContext:
            *opcode = PixOp_EndEvent;
            *eventMetadata |= PIX_EVENT_METADATA_ON_CONTEXT;
            break;

        case PixOp_BeginEvent_OnContext_VarArgs:
            *opcode = PixOp_BeginEvent;
            *eventMetadata |= PIX_EVENT_METADATA_ON_CONTEXT | PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_BeginEvent_OnContext_NoVarArgs:
            *opcode = PixOp_BeginEvent;
            *eventMetadata |= PIX_EVENT_METADATA_ON_CONTEXT | PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_SetMarker_OnContext_VarArgs:
            *opcode = PixOp_SetMarker;
            *eventMetadata |= PIX_EVENT_METADATA_ON_CONTEXT | PIX_EVENT_METADATA_HAS_COLOR;
            break;

        case PixOp_SetMarker_OnContext_NoVarArgs:
            *opcode = PixOp_SetMarker;
            *eventMetadata |= PIX_EVENT_METADATA_ON_CONTEXT | PIX_EVENT_METADATA_HAS_COLOR;
            break;
        }
    }
}

inline PixOp ConvertOnContextEventTypeToV2(PixOp opcode)
{
    switch (opcode)
    {
    case PixOp_EndEvent_OnContext:
        return PixOp_EndEvent;

    case PixOp_BeginEvent_OnContext_VarArgs: __fallthrough;
    case PixOp_BeginEvent_OnContext_NoVarArgs:
        return PixOp_BeginEvent;

    case PixOp_SetMarker_OnContext_VarArgs: __fallthrough;
    case PixOp_SetMarker_OnContext_NoVarArgs:
        return PixOp_SetMarker;
    };

    return opcode;
}

inline PixOp PIXDecodeOpcode(UINT64 eventBits)
{
    // To be used only for V2
    return static_cast<PixOp>((eventBits & PIXEventsTypeReadMask) >> PIXEventsTypeBitShift);
}

inline UINT8 PIXDecodeSize(UINT64 eventBits)
{
    // To be used only for V2
    return static_cast<UINT8>((eventBits & PIXEventsSizeReadMask) >> PIXEventsSizeBitShift);
}

inline UINT64 PIXDecodeIndexColor(UINT8 metadata)
{
    // To be used only for V2
    return (metadata >> 4);
}

static const UINT64 PIXEventsStringEmptyBitsMask = ~(PIXEventsStringAlignmentReadMask
    | PIXEventsStringCopyChunkSizeReadMask
    | PIXEventsStringIsANSIReadMask
    | PIXEventsStringIsShortcutReadMask);

inline bool PIXDecodeStringInfo(UINT64 eventBits, UINT64& alignment, UINT64& copyChunkSize, bool& isANSI, bool& isShortcut)
{
    alignment = (eventBits & PIXEventsStringAlignmentReadMask) >> PIXEventsStringAlignmentBitShift;
    copyChunkSize = (eventBits & PIXEventsStringCopyChunkSizeReadMask) >> PIXEventsStringCopyChunkSizeBitShift;
    isANSI = ((eventBits & PIXEventsStringIsANSIReadMask) >> PIXEventsStringIsANSIBitShift) > 0;
    isShortcut = ((eventBits & PIXEventsStringIsShortcutReadMask) >> PIXEventsStringIsShortcutBitShift) > 0;

    return (eventBits & PIXEventsStringEmptyBitsMask) == 0;
}
