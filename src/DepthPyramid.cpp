#if !RVE_SERVER
#include "DepthPyramid.hpp"
#include "App.hpp"
#include <RGL/Texture.hpp>
#include "RenderEngine.hpp"

namespace RavEngine {
	DepthPyramid::DepthPyramid(uint16_t width, const std::string_view name)
	{
        width = std::min<uint16_t>(width, 1024);
        
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
            .debugName = name.data()
        });
	}
    DepthPyramid::~DepthPyramid()
    {
        auto app = GetApp();
        if (app) {
            app->GetRenderEngine().gcTextures.enqueue(pyramidTexture);
        }
    }
}

#endif
