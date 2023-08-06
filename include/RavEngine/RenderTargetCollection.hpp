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
		glm::mat4 viewProj;
		glm::vec3 camPos;
	};
}