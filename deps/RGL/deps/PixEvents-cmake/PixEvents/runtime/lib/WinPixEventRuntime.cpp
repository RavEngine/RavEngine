// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "WinPixEventRuntime.h"

#include "BlockAllocator.h"
#include "IncludePixEtw.h"
#include "ThreadData.h"
#include "Threads.h"
#include "Worker.h"

#include <d3d12.h> // must be before pix3.h
#include <pix3.h>

#include <wil/resource.h>

namespace WinPixEventRuntime
{
    class EtwWriter
    {
        mutable wil::srwlock m_srwlock;

        Threads m_threads;
        std::unique_ptr<Worker> m_worker = CreateWorker();
        bool m_isEnabled = false;
        
    public:
        EtwWriter() = default;

        ~EtwWriter()
        {
            try
            {
                Flush();
            }
            catch (...)
            {
                // Swallow errors
            }
        }
        

        void RegisterThread(ThreadData* thread)
        {
            auto lock = m_srwlock.lock_exclusive();
            m_threads.Add(thread, m_isEnabled);
        }

        void UnregisterThread(ThreadData* thread)
        {
            auto lock = m_srwlock.lock_exclusive();
            m_threads.Remove(thread);
        }

        void Enable()
        {
            auto lock = m_srwlock.lock_exclusive();

            if (!m_isEnabled)
            {
                m_isEnabled = true;
                m_threads.UpdateThreads(true);
                m_worker->Start();
            }
        }

        void Disable()
        {        
            auto lock = m_srwlock.lock_exclusive();

            if (m_isEnabled)
            {
                m_isEnabled = false;
                m_threads.UpdateThreads(false);
                m_worker->Stop();
            }
        }

        void Flush()
        {
            auto lock = m_srwlock.lock_exclusive();

            if (!m_isEnabled)
                return;

            auto eventTime = PIXGetTimestampCounter();
            
            m_worker->Stop();

            m_threads.Flush(eventTime);
        }

        void TakeBlock(BlockAllocator::Block block)
        {
            auto lock = m_srwlock.lock_exclusive();
            m_worker->Add(std::move(block));
        }
    };


    static std::optional<EtwWriter> g_etwWriter;


    void Initialize() noexcept
    {
        BlockAllocator::Initialize();
        g_etwWriter.emplace();
    }


    void Shutdown() noexcept
    {
        g_etwWriter->Flush();
        g_etwWriter.reset();
        BlockAllocator::Shutdown();
    }


    void EnableCapture() noexcept
    {
        g_etwWriter->Enable();
    }


    void DisableCapture() noexcept
    {
        g_etwWriter->Disable();
    }


    void FlushCapture() noexcept
    {   
        g_etwWriter->Flush();
    }


    void RegisterThread(ThreadData* threadData) noexcept
    {
        g_etwWriter->RegisterThread(threadData);
    }


    void UnregisterThread(ThreadData* threadData) noexcept
    {
        g_etwWriter->UnregisterThread(threadData);
    }



    void TakeBlock(BlockAllocator::Block block) noexcept
    {
        g_etwWriter->TakeBlock(std::move(block));
    }
}

//
// DLL exports used by pix3.h
//


UINT64 PIXEventsReplaceBlock(PIXEventsThreadInfo* threadInfo, bool getEarliestTime) noexcept
{
    std::optional<uint64_t> eventTime;
    if (getEarliestTime)
        eventTime = PIXGetTimestampCounter();

    return WinPixEventRuntime::ThreadData::ReplaceBlock(threadInfo, eventTime);
}


#ifdef PIX_EVENTS_ARE_TURNED_ON
// If events are turned off, these functions are empty inlines in pix3.h

void WINAPI PIXReportCounter(_In_ PCWSTR name, float value)
{
    EventWritePIXReportCounterData(value, name);
}

void WINAPI PIXNotifyWakeFromFenceSignal(_In_ HANDLE event)
{
    EventWritePIXNotifyWakeFromFenceSignalEventData((UINT64)event);
}

void WINAPI PIXRecordMemoryAllocationEvent(USHORT allocatorId, void* baseAddress, size_t size, UINT64 metadata)
{
    EventWritePIXTrackMemoryAllocation(allocatorId, baseAddress, size, metadata);
}

void WINAPI PIXRecordMemoryFreeEvent(USHORT allocatorId, void* baseAddress, size_t size, UINT64 metadata)
{
    EventWritePIXTrackMemoryFree(allocatorId, baseAddress, size, metadata);
}

#endif

DWORD WINAPI PIXGetCaptureState()
{
    return 0;
}


//
// These are exported from the dll to allow open source applications to
// GetProcAddress them without worrying about redistributing the pix3 headers.
//

void WINAPI PIXEndEventOnCommandList(ID3D12GraphicsCommandList* commandList)
{
    PIXEndEvent(commandList);
}

void WINAPI PIXEndEventOnCommandQueue(ID3D12CommandQueue* commandQueue)
{
    PIXEndEvent(commandQueue);
}

void WINAPI PIXBeginEventOnCommandList(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString)
{
    PIXBeginEvent(commandList, color, formatString);
}

void WINAPI PIXBeginEventOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString)
{
    PIXBeginEvent(commandQueue, color, formatString);
}

void WINAPI PIXSetMarkerOnCommandList(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString)
{
    PIXSetMarker(commandList, color, formatString);
}

void WINAPI PIXSetMarkerOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString)
{
    PIXSetMarker(commandQueue, color, formatString);
}
