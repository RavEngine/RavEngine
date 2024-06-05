// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <optional>

struct PEvtBlkHdr;

namespace WinPixEventRuntime::BlockAllocator
{
    void Initialize();
    void Shutdown();

    void Free(PEvtBlkHdr* block);

    struct Deleter { void operator ()(PEvtBlkHdr* block) { Free(block); } };

    using Block = std::unique_ptr<PEvtBlkHdr, Deleter>;

    Block Allocate(std::optional<uint64_t> const& eventTime);

    void WriteBlock(BlockAllocator::Block block);
}
