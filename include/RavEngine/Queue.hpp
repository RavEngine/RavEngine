#pragma once
#include <concurrentqueue.h>
#include <queue>

namespace RavEngine {
    template<typename T>
    using ConcurrentQueue = moodycamel::ConcurrentQueue<T>;

    template<typename T>
    using Queue = std::queue<T>;
}
