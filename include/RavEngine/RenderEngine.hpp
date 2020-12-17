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
		
		static constexpr uint8_t gbufferSize = 4;
		static constexpr uint8_t lightingAttachmentsSize = 2;
		
		bgfx::TextureHandle attachments[gbufferSize];
			//RGBA (A not used in opaque)
			//normal vectors
			//xyz of pixel
			//depth texture
		bgfx::UniformHandle gBufferSamplers[gbufferSize];

		bgfx::FrameBufferHandle gBuffer;	//full gbuffer
		bgfx::FrameBufferHandle lightingBuffer;	//for lighting, shares depth with main
		bgfx::TextureHandle lightingAttachments[lightingAttachmentsSize];
		bgfx::UniformHandle lightingSamplers[lightingAttachmentsSize];
        
        static bgfx::VertexBufferHandle screenSpaceQuadVert;
        static bgfx::IndexBufferHandle screenSpaceQuadInd;
				
		bgfx::FrameBufferHandle createFrameBuffer(bool, bool);
		
		//to reduce magic numbers
		struct Views{
			enum{
				FinalBlit = 0,
				DeferredGeo,
				Lighting,
				Count
			};
		};
		
		struct dim{
			int width, height;
		} dims;
		
		template<typename T>
		inline void DrawLightsOfType(ComponentStore& components){
			//must set before changing shaders
			auto lights = components.GetAllComponentsOfTypeFastPath<T>();
			if (lights.size() == 0){
				return;
			}
			for(int i = 0; i < gbufferSize; i++){
				bgfx::setTexture(i, gBufferSamplers[i], attachments[i]);
			}
			
			const auto stride = T::InstancingStride();
			const auto numLights = lights.size();	//TODO: factor in light frustum culling
			
			//create buffer for GPU instancing
			bgfx::InstanceDataBuffer idb;
			bgfx::allocInstanceDataBuffer(&idb, numLights, stride);
			
			//fill the buffer
			int i = 0;
			for(const Ref<T>& light : lights){	//TODO: factor in light frustum culling
				float* ptr = (float*)(idb.data + i);
				light->AddInstanceData(ptr);
				i += stride;
			}
			
			bgfx::setInstanceDataBuffer(&idb);
			
			//set the required state for this light type
			T::SetState();
			
			//execute instance draw call
			T::Draw(Views::Lighting);	//view 2 is the lighting pass
		}
	
    };
}
