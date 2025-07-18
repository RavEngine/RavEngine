#pragma once
#if !RVE_SERVER
#include <RGL/Types.hpp>
#include <RGL/TextureFormat.hpp>
#include "mathtypes.hpp"
#include "DepthPyramid.hpp"
#include "Vector.hpp"
#include "Layer.hpp"
#include "Function.hpp"

namespace RavEngine{
    class RenderTexture;
    struct PostProcessEffectStack;

	struct ViewportOverride {
		glm::vec2 originFactor {0, 0};
		glm::vec2 sizeFactor {1, 1};
	};

	struct RenderTargetCollection {
		RGLTexturePtr depthStencil, lightingTexture, lightingScratchTexture, mlabDepth, radianceTexture, viewSpaceNormalsTexture, ssaoOutputTexture1, ssaoOutputTexture2, ssgiOutputTexture;
        
        std::array<RGLTexturePtr, 4> mlabAccum;
        constexpr static std::array<RGL::TextureFormat, 4> formats = {RGL::TextureFormat::RGBA16_Sfloat, RGL::TextureFormat::RGBA8_Unorm, RGL::TextureFormat::RGBA8_Unorm, RGL::TextureFormat::RGBA8_Unorm};
        constexpr static auto mlabDepthFormat = RGL::TextureFormat::RGBA16_Sfloat;
		Function<RGL::ITexture*(void)> finalFramebufferFn;
		DepthPyramid depthPyramid;

		void ResolveFinalFB() const{
			if (!finalFB) {
				finalFB = finalFramebufferFn();
			}
		}

		auto GetPresentedFB() const{
			return finalFB;
		}
	private:
		mutable RGL::ITexture* finalFB = nullptr;
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
            float fov;
            uint32_t targetWidth, targetHeight;
            const PostProcessEffectStack* postProcessingEffects = nullptr;
			const void* tonemap = nullptr;	// because we can't forward declare 'using's 
			IndirectLightingSettings indirectSettings;
		};
		Vector<camData> camDatas;
		dim_t<int> pixelDimensions;
	};
}
#endif
