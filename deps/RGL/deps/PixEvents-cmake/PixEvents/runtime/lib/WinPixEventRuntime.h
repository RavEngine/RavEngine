// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// WinPixEventRuntime interface - functions called by either the
// WinPixEventRuntime DLLMain, decoding or test code.

#pragma once

#include "BlockAllocator.h"
#include <shared/PEvtBlk.h>

namespace WinPixEventRuntime
{
    void Initialize() noexcept;
    void Shutdown() noexcept;

    void EnableCapture() noexcept;
    void DisableCapture() noexcept;
    void FlushCapture() noexcept;

    class ThreadData;
    void RegisterThread(ThreadData* threadData) noexcept;
    void UnregisterThread(ThreadData* threadData) noexcept;

    void TakeBlock(BlockAllocator::Block block) noexcept;

    class Worker;
    std::unique_ptr<Worker> CreateWorker() noexcept;
    
    void WriteBlock(uint32_t numBytes, void* block) noexcept;
}


