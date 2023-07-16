#pragma once
#include <RGL/Common.hpp>
#include <cstdint>

namespace RGL{

    enum class SamplerAddressMode : uint8_t {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce
    };

    struct SamplerConfig{
        SamplerAddressMode 
            addressModeU = SamplerAddressMode::Wrap, 
            addressModeV = SamplerAddressMode::Wrap,
            addressModeW = SamplerAddressMode::Wrap;
        float borderColor[4]{0,0,0,1};
        DepthCompareFunction compareFunction = DepthCompareFunction::Always;
    };

    struct ISampler{
    
    };
}
