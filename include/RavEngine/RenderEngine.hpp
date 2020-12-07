//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#pragma once
#include "SharedObject.hpp"
#include "Entity.hpp"
#include "WeakRef.hpp"
#include "Uniform.hpp"
#include <bgfx/bgfx.h>

struct SDL_Window;

namespace RavEngine {
    class SDLSurface;
    class DeferredBlitShader;

    class RenderEngine : public SharedObject {
    public:
        virtual ~RenderEngine();
        RenderEngine();
        void Draw(Ref<World>);

        static const std::string currentBackend();

		static SDL_Window* const GetWindow(){
			return window;
		}

        void resize();
		
		static struct vs {
			int width = 960; int height = 540;
			bool vsync = true;
		} VideoSettings;
		
    protected:
		static SDL_Window* window;
        static void Init();
		static uint32_t GetResetFlags();
		
		bgfx::TextureHandle attachments[4];
			//RGBA (A not used in opaque)
			//normal vectors
			//xyz of pixel
			//depth texture
		bgfx::UniformHandle gBufferSamplers[4];
		
		enum Samplers{
			PBR_ALBEDO_LUT = 0,
			
			PBR_BASECOLOR = 1,
			PBR_METALROUGHNESS = 2,
			PBR_NORMAL = 3,
			PBR_OCCLUSION = 4,
			PBR_EMISSIVE = 5,
			
			LIGHTS_POINTLIGHTS = 6,
			CLUSTERS_CLUSTERS = 7,
			CLUSTERS_LIGHTINDICES = 8,
			CLUSTERS_LIGHTGRID = 9,
			CLUSTERS_ATOMICINDEX = 10,
			
			DEFERRED_DIFFUSE_A = 7,
			DEFERRED_NORMAL = 8,
			DEFERRED_F0_METALLIC = 9,
			DEFERRED_EMISSIVE_OCCLUSION = 10,
			DEFERRED_DEPTH = 11,
		};
		
		const uint8_t gbufferTextureUnits[4] = {Samplers::DEFERRED_DIFFUSE_A, Samplers::DEFERRED_NORMAL, Samplers::DEFERRED_F0_METALLIC, Samplers::DEFERRED_DEPTH};

		bgfx::FrameBufferHandle gBuffer;	//full gbuffer
		bgfx::FrameBufferHandle frameBuffer;
        
        bgfx::VertexBufferHandle screenSpaceQuadVert = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle screenSpaceQuadInd = BGFX_INVALID_HANDLE;
        
        Ref<DeferredBlitShader> blitShader;
		
		bgfx::FrameBufferHandle createFrameBuffer(bool, bool);
    };
}
