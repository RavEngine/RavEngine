// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Threads.h"

#include "ThreadData.h"

namespace WinPixEventRuntime
{
    Threads::Threads() = default;
    Threads::~Threads() = default;

    void Threads::Add(ThreadData* thread, bool isEnabled)
    {
        m_threads.push_back(thread);
        UpdateThread(thread, isEnabled);
    }


    void Threads::Remove(ThreadData* thread)
    {
        if (!m_threads.empty())
        {
            m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(), [=](auto* o) { return o == thread; }));
        }
    }


    void Threads::UpdateThreads(bool isEnabled)
    {
        for (auto* thread : m_threads)
        {
            UpdateThread(thread, isEnabled);
        }
    }


    void Threads::UpdateThread(ThreadData* thread, bool isEnabled)
    {
        thread->SetEnabled(isEnabled);
    }


    void Threads::Flush(uint64_t eventTime) const
    {
        for (auto* thread : m_threads)
        {
            WriteBlock(thread->Flush(eventTime));
        }
    }
}
