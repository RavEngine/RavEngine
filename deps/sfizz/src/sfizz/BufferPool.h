// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Config.h"
#include "Buffer.h"
#include "AudioBuffer.h"
#include "AudioSpan.h"
#include "utility/Debug.h"
#include <array>
#include <memory>
#include <functional>
#include "absl/algorithm/container.h"
#ifndef NDEBUG
#include "MathHelpers.h"
#endif

namespace sfz {

template <class T>
class SpanHolder {
public:
    SpanHolder() {}
    SpanHolder(const SpanHolder<T>&) = delete;
    SpanHolder<T>& operator=(const SpanHolder<T>&) = delete;
    SpanHolder(SpanHolder<T>&& other)
    {
        this->value = other.value;
        this->available = other.available;
        other.available = nullptr;
    }
    SpanHolder<T>& operator=(SpanHolder<T>&& other)
    {
        this->value = other.value;
        this->available = other.available;
        other.available = nullptr;
        return *this;
    }
    SpanHolder(T&& value, int* available)
        : value(std::forward<T>(value))
        , available(available)
    {
    }
    T& operator*() { return value; }
    T* operator->() { return &value; }
    explicit operator bool() const { return available != nullptr; }
    ~SpanHolder()
    {
        if (available)
            *available += 1;
    }

private:
    T value {};
    int* available { nullptr };
};

class BufferPool {
public:
    BufferPool()
    {
        for (auto& buffer : stereoBuffers) {
            buffer.addChannels(2);
        }
        monoAvailable.resize(config::bufferPoolSize);
        stereoAvailable.resize(config::stereoBufferPoolSize);
        indexAvailable.resize(config::indexBufferPoolSize);
        _setBufferSize(config::defaultSamplesPerBlock);
    }

    void setBufferSize(unsigned bufferSize)
    {
        ASSERT(absl::c_all_of(monoAvailable, [](int value) { return value == 1; }));
        ASSERT(absl::c_all_of(indexAvailable, [](int value) { return value == 1; }));
        ASSERT(absl::c_all_of(stereoAvailable, [](int value) { return value == 1; }));
        _setBufferSize(bufferSize);
    }

    SpanHolder<absl::Span<float>> getBuffer(size_t numFrames)
    {
        const auto availableIt = absl::c_find(monoAvailable, 1);
        if (availableIt == monoAvailable.end()) {
            DBG("[sfizz] No free buffers available...");
            return {};
        }
        const auto freeIndex = std::distance(monoAvailable.begin(), availableIt);

        if (monoBuffers[freeIndex].size() < numFrames) {
            DBG("[sfizz] Someone asked for a buffer of size " << numFrames << "; only " << monoBuffers[freeIndex].size() << " available...");
            return {};
        }

#ifndef NDEBUG
        maxBuffersUsed = 1 + absl::c_count_if(monoAvailable, [](int value) { return value == 0; });
#endif
        *availableIt -= 1;
        return { absl::MakeSpan(monoBuffers[freeIndex]).first(numFrames), &*availableIt };
    }

    SpanHolder<absl::Span<int>> getIndexBuffer(size_t numFrames)
    {
        const auto availableIt = absl::c_find(indexAvailable, 1);
        if (availableIt == indexAvailable.end()) {
            DBG("[sfizz] No available index buffers in the pool");
            return {};
        }
        const auto freeIndex = std::distance(indexAvailable.begin(), availableIt);

        if (indexBuffers[freeIndex].size() < numFrames) {
            DBG("[sfizz] Someone asked for a index buffer of size " << numFrames << "; only " << indexBuffers[freeIndex].size() << " available...");
            return {};
        }

#ifndef NDEBUG
        maxIndexBuffersUsed = 1 + absl::c_count_if(indexAvailable, [](int value) { return value == 0; });
#endif
        *availableIt -= 1;
        return { absl::MakeSpan(indexBuffers[freeIndex]).first(numFrames), &*availableIt };
    }

    SpanHolder<AudioSpan<float>> getStereoBuffer(size_t numFrames)
    {
        const auto availableIt = absl::c_find(stereoAvailable, 1);
        if (availableIt == stereoAvailable.end()) {
            DBG("[sfizz] No available stereo buffers in the pool");
            return {};
        }
        const auto freeIndex = std::distance(stereoAvailable.begin(), availableIt);

        if (stereoBuffers[freeIndex].getNumFrames() < numFrames) {
            DBG("[sfizz] Someone asked for a stereo buffer of size " << numFrames << "; only " << stereoBuffers[freeIndex].getNumFrames() << " available...");
            return {};
        }

#ifndef NDEBUG
        maxStereoBuffersUsed = 1 + absl::c_count_if(stereoAvailable, [](int value) { return value == 0; });
#endif
        *availableIt -= 1;
        return { sfz::AudioSpan<float>(stereoBuffers[freeIndex]).first(numFrames), &*availableIt };
    }

#ifndef NDEBUG
    ~BufferPool()
    {
        // DBG("Max buffers used: " << maxBuffersUsed);
        // DBG("Max index buffers used: " << maxIndexBuffersUsed);
        // DBG("Max stereo buffers used: " << maxStereoBuffersUsed);
    }
#endif

private:
    void _setBufferSize(unsigned bufferSize)
    {
        for (auto& buffer : monoBuffers) {
            buffer.resize(bufferSize);
        }

        for (auto& buffer : indexBuffers) {
            buffer.resize(bufferSize);
        }

        for (auto& buffer : stereoBuffers) {
            buffer.resize(bufferSize);
        }

        absl::c_fill(monoAvailable, 1);
        absl::c_fill(stereoAvailable, 1);
        absl::c_fill(indexAvailable, 1);
    }

    std::array<sfz::Buffer<float>, config::bufferPoolSize> monoBuffers;
    std::vector<int> monoAvailable;
    std::array<sfz::Buffer<int>, config::bufferPoolSize> indexBuffers;
    std::vector<int> indexAvailable;
    std::array<sfz::AudioBuffer<float>, config::stereoBufferPoolSize> stereoBuffers;
    std::vector<int> stereoAvailable;
#ifndef NDEBUG
    mutable int maxBuffersUsed { 0 };
    mutable int maxIndexBuffersUsed { 0 };
    mutable int maxStereoBuffersUsed { 0 };
#endif
};
}
