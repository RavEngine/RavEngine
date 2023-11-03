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

    enum class MinMagFilterMode : uint8_t {
        Nearest,
        Linear
    };

    enum class MipFilterMode : uint8_t{
        NotMipped, 
        Nearest,
        Linear
    };

    struct SamplerConfig{
        SamplerAddressMode 
            addressModeU = SamplerAddressMode::Wrap, 
            addressModeV = SamplerAddressMode::Wrap,
            addressModeW = SamplerAddressMode::Wrap;
        float borderColor[4]{0,0,0,1};
        DepthCompareFunction compareFunction = DepthCompareFunction::Always;
        MinMagFilterMode
            minFilter = MinMagFilterMode::Nearest,
            magFilter = MinMagFilterMode::Nearest;
        MipFilterMode mipFilter = MipFilterMode::NotMipped;
    };

    struct ISampler{
    
    };
}
