#pragma once
#include <limits>

namespace RavEngine{
    using renderlayer_t = uint32_t;
    constexpr renderlayer_t ALL_LAYERS = std::numeric_limits<decltype(ALL_LAYERS)>::max();

    using perobject_t = uint16_t;
    constexpr perobject_t FrustumCullingBit = 1;
    constexpr perobject_t OcclusionCullingBit = 1 << 1;
    constexpr perobject_t CastsShadowsBit = 1 << 2;
    constexpr perobject_t RecievesShadowsBit = 1 << 3;
    constexpr perobject_t ALL_ATTRIBUTES = std::numeric_limits<decltype(ALL_ATTRIBUTES)>::max();


    struct IndirectLightingSettings {
        float ssaoStrength = 1;
        bool SSAOEnabled = true;
        bool SSGIEnabled = true;
    };
}
