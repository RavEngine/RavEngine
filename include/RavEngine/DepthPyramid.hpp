#if !RVE_SERVER
#pragma once
#include <RGL/Types.hpp>

namespace RavEngine {

	struct DepthPyramid {
		DepthPyramid(uint32_t width);
		DepthPyramid() {}
		RGLTexturePtr pyramidTexture = nullptr;
		uint32_t dim = 0;	// size of mip 0
		uint32_t numLevels = 0;
	};
}
#endif