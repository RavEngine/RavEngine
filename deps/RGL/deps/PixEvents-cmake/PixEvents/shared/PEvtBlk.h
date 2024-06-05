// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <windows.h>

// This is the legacy file format data that we need to remain compatible with.


// Indicates the type of data in the block
enum PIXEVT_BLOCK_TYPE : UINT32
{
    PIXEVT_CPU_BLOCK,

    PIXEVT_INVALID_BLOCK = (UINT32)-1
};

// Header fields for PIXEVT_CPU_BLOCK blocks
struct PEvtCpuBlkHdr
{
    UINT32 threadId;                // From Win32 GetThreadId
    UINT32 processId;               // From Win32 GetCurrentProcessId
    UINT64 beginTimestamp;          // Full timestamp (QPC on Windows) of first event in this block
    UINT64 endTimestamp;            // Full timestamp (QPC on Windows) of last event in this block
};

// PEvtBlkHdr: 
// PIX Event Data block, exclusively owned by single user mode thread or D3D device context
struct PEvtBlkHdr
{
    BYTE*  pPIXLimit;               // Points to end of the block
    BYTE*  pPIXCurrent;             // Current insertion point for incoming data
    UINT32 Reserved;                // For padding (64-bit alignment) and potential future use
    PIXEVT_BLOCK_TYPE BlockType;    // Whether this block contains CPU info, GPU info, etc.
    PEvtCpuBlkHdr cpuHeader;    // CPU-specific block header info
};

