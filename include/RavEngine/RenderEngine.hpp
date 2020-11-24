//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include "Entity.hpp"
#include "WeakRef.hpp"
#include <bgfx/bgfx.h>

struct SDL_Window;

namespace RavEngine {
    class SDLSurface;

    class RenderEngine : public SharedObject {
    public:
        virtual ~RenderEngine();
        RenderEngine();
        void Draw(Ref<World>);

        static const std::string currentBackend();

		static SDL_Window* const GetWindow(){
			return window;
		}
		
		/**
		 Determines if the current hardware can support deferred rendering
		 @return true if the current hardware supports deferred
		 */
		static bool DeferredSupported();

        void resize();
		
		static struct vs {
			int width = 960; int height = 540;
			bool vsync = true;
		} VideoSettings;
		
    protected:
		static SDL_Window* window;
        static void Init();
		static uint32_t GetResetFlags();
		
		enum GBufferAttachment : size_t
		{
			// no world position
			// gl_Fragcoord is enough to unproject
			
			// RGB = diffuse
			// A = a (remapped roughness)
			Diffuse_A,
			
			// RG = encoded normal
			Normal,
			
			// RGB = F0 (Fresnel at normal incidence)
			// A = metallic
			// TODO? don't use F0, calculate from diffuse and metallic in shader
			//       where do we store metallic?
			F0_Metallic,
			
			// RGB = emissive radiance
			// A = occlusion multiplier
			EmissiveOcclusion,
			
			Depth,
			
			Count
		};
		
		//some code adapted from: https://github.com/pezcode/Cluster/blob/master/src/Renderer/DeferredRenderer.h
		static constexpr uint64_t gBufferSamplerFlags =
			BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
			BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP |
			BGFX_SAMPLER_V_CLAMP;
		
		static constexpr bgfx::TextureFormat::Enum gBufferAttachmentFormats[GBufferAttachment::Count - 1] = {
			bgfx::TextureFormat::BGRA8,
			bgfx::TextureFormat::RG16F,
			bgfx::TextureFormat::BGRA8,
			bgfx::TextureFormat::BGRA8
			// depth format is determined dynamically
		};
		
		struct TextureBuffer
		{
			 bgfx::TextureHandle handle;
			const char* name;
		};
		
		//framebuffers for gbuffers
		TextureBuffer gBufferTextures[GBufferAttachment::Count + 1]; // includes depth, + null-terminated
		uint8_t gBufferTextureUnits[GBufferAttachment::Count];
		static constexpr char* gBufferSamplerNames[GBufferAttachment::Count] = { "s_texDiffuseA", "s_texNormal", "s_texF0Metallic", "s_texEmissiveOcclusion", "s_texDepth"};
		bgfx::UniformHandle gBufferSamplers[GBufferAttachment::Count];
		
		bgfx::FrameBufferHandle gBuffer = BGFX_INVALID_HANDLE;
		
		bgfx::TextureHandle lightDepthTexture = BGFX_INVALID_HANDLE;
		bgfx::FrameBufferHandle accumFrameBuffer = BGFX_INVALID_HANDLE;
		bgfx::FrameBufferHandle frameBuffer = BGFX_INVALID_HANDLE;
		
		bgfx::UniformHandle lightIndexVecUniform = BGFX_INVALID_HANDLE;
		
		bgfx::ProgramHandle geometryProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle fullscreenProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle pointLightProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle transparencyProgram = BGFX_INVALID_HANDLE;
		
		/**
		 Create a gbuffer to use in the deferred renderer
		 @return bgfx handle to the frame buffer
		 */
		static bgfx::FrameBufferHandle createGBuffer();
		
		/**
		 Bind the gbuffer member of this class
		 */
		void bindGBuffer();
		
		/**
		 Determines the texture format to use given parameters
		 @param textureFlags the bitfield describing texture properties
		 @param stencil
		 @return which texture format is appropriate for the supplied paramters
		 */
		static bgfx::TextureFormat::Enum findDepthFormat(uint64_t textureFlags, bool stencil = false);
		
		/**
		 Create a FrameBuffer
		 @param hdr true to support high dynamic range
		 @param depth true to support depth
		 @return handle to created frame buffer
		 */
		static bgfx::FrameBufferHandle createFrameBuffer(bool hdr = true, bool depth = true);
    };
}
