#pragma once
#include <array>
#include <cstdint>

namespace RavEngine {
    template<typename T, size_t N>
    using Array = std::array<T,N>;
}
