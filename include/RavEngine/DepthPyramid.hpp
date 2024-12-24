#if !RVE_SERVER
#pragma once
#include <RGL/Types.hpp>
#include <string_view>

namespace RavEngine {

	struct DepthPyramid {
		DepthPyramid(uint16_t width, const std::string_view name = "Depth Pyramid");
		DepthPyramid() {}
		~DepthPyramid();
		RGLTexturePtr pyramidTexture = nullptr;
        uint16_t dim = 0;	// size of mip 0
        uint8_t numLevels = 0;
	};
}
#endif
