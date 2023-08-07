#pragma once
#include <RGL/Types.hpp>
#include "mathtypes.hpp"

namespace RavEngine{
	struct RenderTargetCollection {
		RGLTexturePtr diffuseTexture, normalTexture, depthStencil, lightingTexture;
		RGL::ITexture* finalFramebuffer;
	};

	struct RenderViewCollection {
		RenderTargetCollection collection;
		struct camData {
			glm::mat4 viewProj;
			glm::vec3 camPos;

			struct ViewportOverride {
				glm::vec2 originFactor {0, 0};
				glm::vec2 sizeFactor {0, 0};
			} viewportOverride;
		};
		std::vector<camData> camDatas;
		dim_t<int> pixelDimensions;
	};
}