// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Worker.h"

#include <wil/resource.h>

#include <atomic>
#include <thread>
#include <vector>


namespace WinPixEventRuntime
{    
    class ThreadedWorker final : public Worker
    {
        wil::srwlock m_srwlock;
        wil::condition_variable m_cv;

        std::thread m_worker;
        std::atomic<bool> m_requestStop = true;

        std::vector<BlockAllocator::Block> m_pendingBlocks;
        std::vector<BlockAllocator::Block> m_pendingBlocksBackBuffer;

    public:
        ThreadedWorker();
        virtual ~ThreadedWorker() override;

        virtual void Start() override;
        virtual void Stop() override;
        virtual void Add(BlockAllocator::Block block) override;

    private:
        void DoStart();
        
        void Worker();
    };    
}
