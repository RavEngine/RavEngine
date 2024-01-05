#if !RVE_SERVER
#include "DepthPyramid.hpp"
#include "App.hpp"
#include <RGL/Texture.hpp>
#include "RenderEngine.hpp"

namespace RavEngine {
	DepthPyramid::DepthPyramid(uint32_t width)
	{
        // the depth pyramid is the next POT smaller
        dim = pow(2, std::floor(std::log2f(width)));;

        auto device = GetApp()->GetDevice();

        numLevels = 1 + std::floor(std::log2(dim));

        pyramidTexture = device->CreateTexture({
            .usage = {.Sampled = true, .Storage = true, .ColorAttachment = true},
            .aspect = {.HasColor = true },
            .width = dim,
            .height = dim,
            .mipLevels = numLevels,
            .format = RenderEngine::depthPyramidFormat,
            .debugName = "Depth Pyramid Texture"
        });
	}
}

#endif