#pragma once
#include <limits>

namespace RavEngine{
    using renderlayer_t = uint32_t;
    constexpr static renderlayer_t ALL_LAYERS = std::numeric_limits<decltype(ALL_LAYERS)>::max();
}
