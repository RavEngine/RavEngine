// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <vector>

namespace WinPixEventRuntime
{
    class ThreadData;

    class Threads
    {
        std::vector<ThreadData*> m_threads;

    public:
        Threads();
        ~Threads();

        void Add(ThreadData* thread, bool isEnabled);
        void Remove(ThreadData* thread);
        void UpdateThreads(bool isEnabled);
        void Flush(uint64_t eventTime) const;

    private:
        void UpdateThread(ThreadData* thread, bool isEnabled);
    };
}
