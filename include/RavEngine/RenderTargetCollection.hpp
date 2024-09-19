#pragma once
#if !RVE_SERVER
#include <RGL/Types.hpp>
#include "mathtypes.hpp"
#include "DepthPyramid.hpp"
#include "Vector.hpp"
#include "Layer.hpp"

namespace RavEngine{
	struct ViewportOverride {
		glm::vec2 originFactor {0, 0};
		glm::vec2 sizeFactor {1, 1};
	};

	struct RenderTargetCollection {
		RGLTexturePtr normalTexture, depthStencil, lightingTexture, lightingScratchTexture, ssaoTexture, transparencyAccumulation, transparencyRevealage;
		RGL::ITexture* finalFramebuffer;
		DepthPyramid depthPyramid;
	};

	struct RenderViewCollection {
		RenderTargetCollection collection;
		struct camData {
			glm::mat4 viewProj;
			glm::mat4 projOnly;
			glm::mat4 viewOnly;
			glm::vec3 camPos;
			glm::vec2 zNearFar;
			ViewportOverride viewportOverride;
            renderlayer_t layers;
		};
		Vector<camData> camDatas;
		dim_t<int> pixelDimensions;
	};
}
#endif
