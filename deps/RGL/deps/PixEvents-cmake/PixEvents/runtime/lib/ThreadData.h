// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "BlockAllocator.h"

#include <atomic>
#include <optional>
#include <thread>


namespace WinPixEventRuntime
{
    class ThreadData
    {
        PIXEventsThreadInfo m_pixEventsThreadInfo = {};
        BlockAllocator::Block m_currentBlock;
        std::atomic<bool> m_isEnabled = false;
        
        static_assert(std::atomic<bool>::is_always_lock_free);
#if DBG
        std::thread::id m_threadId;
#endif

    public:
        ThreadData();
        ~ThreadData();

        PIXEventsThreadInfo* GetPixEventsThreadInfo();

        static uint64_t ReplaceBlock(PIXEventsThreadInfo* threadInfo, std::optional<uint64_t> const& eventTime);

        void SetEnabled(bool isEnabled);
        BlockAllocator::Block Flush(std::optional<uint64_t> const& eventTime);

    private:
        static ThreadData* GetFromThreadInfo(PIXEventsThreadInfo* threadInfo);
        uint64_t ReplaceBlock(std::optional<uint64_t> const& eventTime);
    };
}
