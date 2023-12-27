#pragma once
#if !RVE_SERVER
#include <RGL/Types.hpp>
#include "mathtypes.hpp"

namespace RavEngine{
	struct ViewportOverride {
		glm::vec2 originFactor {0, 0};
		glm::vec2 sizeFactor {1, 1};
	};

	struct RenderTargetCollection {
		RGLTexturePtr diffuseTexture, normalTexture, roughnessSpecularMetallicAOTexture, depthStencil, depthPyramidTexture, lightingTexture, lightingScratchTexture, ssaoTexture;
		RGL::ITexture* finalFramebuffer;
		uint32_t pyramidSize = 0;			// square texture
		uint32_t numPyramidLevels = 0;
	};

	struct RenderViewCollection {
		RenderTargetCollection collection;
		struct camData {
			glm::mat4 viewProj;
			glm::mat4 projOnly;
			glm::mat4 viewOnly;
			glm::vec3 camPos;

			ViewportOverride viewportOverride;
		};
		std::vector<camData> camDatas;
		dim_t<int> pixelDimensions;
	};
}
#endif
