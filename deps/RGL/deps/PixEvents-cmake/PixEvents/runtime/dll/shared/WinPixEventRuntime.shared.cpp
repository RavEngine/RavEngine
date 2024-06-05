// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <d3d12.h> // must be before pix3.h
#include <pix3.h>

#include <lib/IncludePixEtw.h>
#include <lib/ThreadData.h>
#include <lib/ThreadedWorker.h>
#include <lib/WinPixEventRuntime.h>

//
// These are in 'shared' rather than lib' because we want to be able to replace
// them in the unit tests.
//

PIXEventsThreadInfo* WINAPI PIXGetThreadInfo() noexcept
{
    static thread_local WinPixEventRuntime::ThreadData thisThreadData;
    
    return thisThreadData.GetPixEventsThreadInfo();
}


std::unique_ptr<WinPixEventRuntime::Worker> WinPixEventRuntime::CreateWorker() noexcept
{
    return std::make_unique<WinPixEventRuntime::ThreadedWorker>();
}


void WinPixEventRuntime::WriteBlock(uint32_t numBytes, void* block) noexcept
{
    static std::atomic<uint32_t> eventId = 0u;

    EventWritePIXRecordTimingBlock_v2(eventId.fetch_add(1), numBytes, static_cast<BYTE*>(block));
}
