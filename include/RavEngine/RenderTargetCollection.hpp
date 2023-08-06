#pragma once
namespace RavEngine{
	struct RenderTargetCollection {
		RGLTexturePtr diffuseTexture, normalTexture, depthStencil, lightingTexture;
		RGL::ITexture* finalFramebuffer;
	};
}