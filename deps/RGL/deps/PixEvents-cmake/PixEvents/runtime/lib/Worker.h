// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "BlockAllocator.h"

namespace WinPixEventRuntime
{
    // Abstract base class for the worker.  This allows us to have the threaded worker in
    // the real code and a more deterministic worker in the test code.
    //
    // Note: in release builds the optimizer is able to devirtualize all the virtual functions
    // here so they're zero overhead (in fact, they're mostly inlined).
    class Worker
    {
    public:
        virtual ~Worker() = default;
        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void Add(BlockAllocator::Block block) = 0;
    };

}
