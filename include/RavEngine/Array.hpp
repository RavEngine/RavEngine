#pragma once
#include <array>
#include <cstddef>

namespace RavEngine {
    /**
    * A fixed-size array class.
    */
    template<typename T, size_t N>
    using Array = std::array<T,N>;
}
