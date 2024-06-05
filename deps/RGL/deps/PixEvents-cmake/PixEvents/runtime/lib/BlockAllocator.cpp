// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "BlockAllocator.h"

#include "WinPixEventRuntime.h"

#include <pix3.h>

#include <wil/resource.h>

#include <optional>

#include <assert.h>

namespace WinPixEventRuntime::BlockAllocator
{
    static constexpr size_t BLOCK_SIZE = 16 * 1024; // 16kb block size

    class BlockAllocator
    {
        wil::srwlock m_srwlock;
        HANDLE m_heap;
        DWORD m_createHeapStatus;

    public:
        BlockAllocator()
            : m_heap(nullptr)
            , m_createHeapStatus(ERROR_SUCCESS)
        {
        }

        ~BlockAllocator()
        {
            // We don't expect there to be contention for this lock, but assert
            // if there is to give us a chance to detect it. We also take the
            // lock out anyway so if there is a bug here we can mask it in
            // production.
            auto tryLock = m_srwlock.try_lock_exclusive();
            assert(tryLock);
            auto lock = tryLock ? std::move(tryLock) : m_srwlock.lock_exclusive();
            
            if (m_heap)
            {
                HeapDestroy(m_heap);
            }
        }

        void* Allocate()
        {
            auto lock = m_srwlock.lock_exclusive();

            // If we failed to create the heap before then it's not going to succeed later
            if (m_createHeapStatus != ERROR_SUCCESS)
                return nullptr;

            // We lazy create the heap on first allocate
            if (!m_heap)
            {
                // We have our own lock
                auto options = HEAP_NO_SERIALIZE;

                // Initial size is for some arbitrary number of blocks
                auto initialSize = BLOCK_SIZE * 16;

                // The heap can grow if necessary
                auto maximumSize = 0;

                m_heap = HeapCreate(options, initialSize, maximumSize);

                if (!m_heap)
                {
                    m_createHeapStatus = GetLastError();
                    return nullptr;
                }
            }

            return HeapAlloc(m_heap, HEAP_NO_SERIALIZE, BLOCK_SIZE);
        }

        void Free(void* p)
        {
            if (!p)
                return;

            auto lock = m_srwlock.lock_exclusive();
            auto result = HeapFree(m_heap, HEAP_NO_SERIALIZE, p);
            if (!result)
            {
                auto gle = GetLastError();
                (void)gle;
                assert(result && !gle);
            }
            (void)result;
        }
    };


    static std::optional<BlockAllocator> g_blockAllocator;

    void Initialize()
    {
        g_blockAllocator.emplace();
    }


    void Shutdown()
    {
        g_blockAllocator.reset();
    }


    Block Allocate(std::optional<uint64_t> const& eventTime)
    {
        PEvtBlkHdr* block = static_cast<PEvtBlkHdr*>(g_blockAllocator->Allocate());
        if (!block)
            return nullptr;

        *block = {};
        block->pPIXLimit = reinterpret_cast<uint8_t*>(block) + BLOCK_SIZE;
        block->pPIXCurrent = reinterpret_cast<uint8_t*>(block + 1);

        block->BlockType = PIXEVT_CPU_BLOCK;
        block->cpuHeader.processId = GetCurrentProcessId();
        block->cpuHeader.threadId = GetCurrentThreadId();
        block->cpuHeader.beginTimestamp = eventTime ? *eventTime : PIXGetTimestampCounter();
        block->cpuHeader.endTimestamp = ~0ull;
        
        return Block(block);
    }


    void Free(PEvtBlkHdr* block)
    {
        if (block)
            g_blockAllocator->Free(block);
    }


    void WriteBlock(Block block)
    {
        if (block)
        {
            WinPixEventRuntime::WriteBlock(block->pPIXLimit - (BYTE*)block.get(), block.get());
        }
    }
}
