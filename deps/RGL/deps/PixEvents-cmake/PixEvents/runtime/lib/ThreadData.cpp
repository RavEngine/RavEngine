// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ThreadData.h"

#include "BlockAllocator.h"
#include "WinPixEventRuntime.h"

#include <pix3.h>

#include <assert.h>

namespace WinPixEventRuntime
{
    /*static*/ ThreadData* ThreadData::GetFromThreadInfo(PIXEventsThreadInfo* threadInfo)
    {
        // We're only going to ever hand out PIXEventsThreadInfo objects that
        // are embedded inside ThreadData objects, so we can get one from the
        // other.
        static_assert(offsetof(ThreadData, m_pixEventsThreadInfo) == 0);
        return reinterpret_cast<ThreadData*>(threadInfo);
    }

    ThreadData::ThreadData()        
    {
#if DBG
        m_threadId = std::this_thread::get_id();
        assert(GetFromThreadInfo(&m_pixEventsThreadInfo) == this);
#endif

        WinPixEventRuntime::RegisterThread(this);
    }

    ThreadData::~ThreadData()
    {
        if (auto oldBlock = Flush(PIXGetTimestampCounter()))
        {
            WinPixEventRuntime::TakeBlock(std::move(oldBlock));
        }
        WinPixEventRuntime::UnregisterThread(this);
    }

    PIXEventsThreadInfo* ThreadData::GetPixEventsThreadInfo()
    {
        // This is our thread info that's only meant to be used by this thread.
        assert(m_threadId == std::this_thread::get_id());        

        if (m_isEnabled)
        {
            if (!m_pixEventsThreadInfo.biasedLimit)
            {
                // This must be the first time that GetPixEventsThreadInfo has
                // been called while enabled. Setting biasedLimit and
                // destination to non-null will trigger an allocation.
                m_pixEventsThreadInfo.biasedLimit = reinterpret_cast<uint64_t*>(~0ull);
                m_pixEventsThreadInfo.destination = reinterpret_cast<uint64_t*>(~0ull);                
            }
        }
        else
        {
            // As the capture is disabled we want to ensure that nothing gets
            // written. If we still find there's a block here we need to free
            // it. If we get here and it hasn't been written, it'll be too late
            // to send it since the ETW provider has been disabled.
            if (m_pixEventsThreadInfo.block || m_currentBlock)
            {
                m_currentBlock.reset();
                m_pixEventsThreadInfo.block = nullptr;
            }

            // This indicates that capture is disabled, and so the entry points
            // (eg PIXBeginEvent) won't attempt to allocate in this state.
            m_pixEventsThreadInfo.biasedLimit = nullptr;
            m_pixEventsThreadInfo.destination = nullptr;
        }

        return &m_pixEventsThreadInfo;
    }


    /*static*/ uint64_t ThreadData::ReplaceBlock(PIXEventsThreadInfo* threadInfo, std::optional<uint64_t> const& eventTime)
    {
        return GetFromThreadInfo(threadInfo)->ReplaceBlock(eventTime);
    }

    uint64_t ThreadData::ReplaceBlock(std::optional<uint64_t> const& eventTime)
    {
        // This is our thread info that's only meant to be used by this thread.
        assert(m_threadId == std::this_thread::get_id());

        if (auto oldBlock = Flush(eventTime))
        {
            TakeBlock(std::move(oldBlock));
        }

        assert(!m_currentBlock);

        // Get and use a new one
        m_currentBlock = BlockAllocator::Allocate(eventTime);
        if (!m_currentBlock)
        {
            // We failed to allocate a new block
            return 0;
        }

        // m_currentBlock points to a block of memory that's expected to start with a PEvtBlkHdr
        // and then contains our own data after that.

        m_pixEventsThreadInfo.block = reinterpret_cast<PIXEventsBlockInfo*>(m_currentBlock.get());
        m_pixEventsThreadInfo.destination = reinterpret_cast<uint64_t*>(m_currentBlock->pPIXCurrent);
        m_pixEventsThreadInfo.biasedLimit = reinterpret_cast<uint64_t*>(m_currentBlock->pPIXLimit) - PIXEventsReservedRecordSpaceQwords;

        return m_currentBlock->cpuHeader.beginTimestamp;        
    }


    BlockAllocator::Block ThreadData::Flush(std::optional<uint64_t> const& eventTime)
    {
        // !!! Potentially unsafe access to m_pixEventsThreadInfo
        // !!! This function might be called from an arbitrary thread, while the
        // !!! thread associated with this ThreadData might be in ReplaceBlock.

        // Hand our current block off so it can be written to disk
        if (m_pixEventsThreadInfo.block)
        {
            assert(m_currentBlock);

            m_currentBlock->cpuHeader.endTimestamp = eventTime ? *eventTime : PIXGetTimestampCounter();

            m_pixEventsThreadInfo = {};
        }
        else
        {
            assert(!m_currentBlock);
        }

        return std::move(m_currentBlock);
    }


    void ThreadData::SetEnabled(bool isEnabled)
    {
        // We take note of this here, but only really respond to it the next time
        // GetPixEventsThreadInfo is called.
        m_isEnabled = isEnabled;
    }
}

