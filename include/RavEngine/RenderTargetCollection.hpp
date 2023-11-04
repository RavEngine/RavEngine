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
		RGLTexturePtr diffuseTexture, normalTexture, roughnessSpecularMetallicAOTexture, depthStencil, depthPyramidTexture, lightingTexture;
		RGL::ITexture* finalFramebuffer;
	};

	struct RenderViewCollection {
		RenderTargetCollection collection;
		struct camData {
			glm::mat4 viewProj;
			glm::vec3 camPos;

			ViewportOverride viewportOverride;
		};
		std::vector<camData> camDatas;
		dim_t<int> pixelDimensions;
	};
}
#endif
