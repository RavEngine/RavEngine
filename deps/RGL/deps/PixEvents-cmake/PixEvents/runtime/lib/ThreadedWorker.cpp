// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ThreadedWorker.h"

namespace WinPixEventRuntime
{    
    ThreadedWorker::ThreadedWorker() = default;

    ThreadedWorker::~ThreadedWorker()
    {
        try
        {
            Stop();
        }
        catch (...)
        {
            // Swallow errors
        }
    }


    void ThreadedWorker::Start()
    {
        // With multiple calling threads trying to Start/Stop/Add it's possible for an existing
        // worker thread to already be running. If it is we want to have it gracefully finish
        // execution before we create another one.
        if (m_worker.joinable())
        {
            m_requestStop = true;
            m_cv.notify_all();
            m_worker.join();
        }

        auto lock = m_srwlock.lock_exclusive();
        DoStart();
    }


    void ThreadedWorker::DoStart()
    {
        m_requestStop = false;
        m_worker = std::thread(
            [=] {
                (void)SetThreadDescription(GetCurrentThread(), L"PixEvent worker");
                Worker(); 
            }
        );
    }


    void ThreadedWorker::Stop()
    {
        auto lock = m_srwlock.lock_exclusive();

        if (m_worker.joinable())
        {
            m_requestStop = true;
            m_cv.notify_all();
            lock.reset();

            m_worker.join();

            // Write out any other blocks that managed to get added
            lock = m_srwlock.lock_exclusive();

            for (auto& block : m_pendingBlocks)
            {
                WriteBlock(std::move(block));
            }
            m_pendingBlocks.clear();
        }
    }


    void ThreadedWorker::Add(BlockAllocator::Block block)
    {
        auto lock = m_srwlock.lock_exclusive();

        m_pendingBlocks.push_back(std::move(block));
        m_cv.notify_all();

        if (m_requestStop)
        {
            if (m_worker.joinable())
            {
                lock.reset();
                m_worker.join();

                Start();
            }
            else
            {
                DoStart();
            }
        }
    }


    void ThreadedWorker::Worker()
    {
        auto lock = m_srwlock.lock_exclusive();

        do
        {
            // We work from m_pendingBlocksBackBuffer.  We persist both of these so that
            // we don't need to reallocate memory for them.
            std::swap(m_pendingBlocks, m_pendingBlocksBackBuffer);
            lock.reset();

            for (auto& block : m_pendingBlocksBackBuffer)
            {
                WriteBlock(std::move(block));
            }
            m_pendingBlocksBackBuffer.clear();

            lock = m_srwlock.lock_exclusive();
            
            while (!m_requestStop && m_pendingBlocks.empty())
            {
                m_cv.wait(lock);
            }
            
        } while (!m_requestStop);
    }
}
