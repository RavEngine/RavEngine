#pragma once
//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Ref.hpp"
#include "Entity.hpp"
#include "WeakRef.hpp"
#include "Uniform.hpp"
#include <bgfx/bgfx.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <fmt/format.h>
#include "FrameData.hpp"

struct SDL_Window;

namespace RavEngine {
    class DeferredBlitShader;
	class GUIMaterialInstance;
	class InputManager;

    class RenderEngine : public Rml::SystemInterface, public Rml::RenderInterface {
	private:
		struct dim{
			int width, height;
		} bufferdims, windowdims;
		
    public:
        virtual ~RenderEngine();
        RenderEngine();

		//render a world, for internal use only
		void Draw(Ref<World>);

		//get reset bitmask, for internal use only
		static uint32_t GetResetFlags();

        /**
         Signal the render thread to stop, and wait until it has stopped. For internal use only.
         */
		void BlockUntilFinishDraw();
        
        /**
         Print a string to the debug text overlay. Stubbed in release.
         Debug messages persist until they are overwritten or cleared.
         @param row the row to display the text
         */
        template<typename ... T>
        static void DebugPrint(uint16_t row, uint8_t color, const std::string& formatstr, T... args){
#ifdef _DEBUG
            dbgmtx.lock();
            debugprints[row] = {fmt::format(formatstr, args...),color};
            dbgmtx.unlock();
#endif
        }
        
        /**
         Clear a debug print message. Stubbed in release.
         @param row the row to clear
         */
        static void ClearDebugPrint(uint16_t row){
#ifdef _DEBUG
            dbgmtx.lock();
            debugprints.erase(row);
            dbgmtx.unlock();
#endif
        }
        
        /**
         Clear all debug print messages. Stubbed in release.
         @param row the row to clear
         */
        static void ClearAllDebugPrint(){
#ifdef _DEBUG
            dbgmtx.lock();
            debugprints.clear();
            dbgmtx.unlock();
#endif
        }

        /**
         @return The name of the current rendering API in use
         */
        static const std::string currentBackend();

		static SDL_Window* const GetWindow(){
			return window;
		}

        /**
         @return the current frame rate using the frame time
         */
		float GetCurrentFPS();
        
        /**
         @return the time in miliseconds to render the last frame
         */
		float GetLastFrameTime();
		
		/**
		 @return the current window buffer size, in pixels
		 */
		const dim& GetBufferSize(){
			return bufferdims;
		}
		
		const dim& GetWindowSize(){
			return windowdims;
		}
		
        /**
         @return the High DPI scale factor. Only applicable on macOS.
         */
		float GetDPIScale() const{
			return (float)bufferdims.width / windowdims.width;
		}

        /**
         Trigger a resize. For internal use only.
         */
        void resize();
		
		static struct vs {
			int width = 960; int height = 540;
			bool vsync = true;
		} VideoSettings;
		
        /**
         Apply changes to the video settings structure
         */
		void SyncVideoSettings();
		
		//to reduce magic numbers
		struct Views{
			enum{
				FinalBlit = 0,
				DeferredGeo,
				Lighting,
				Count
			};
		};

		// Signal to the current renderer what it should draw next 
		void DrawNext(Ref<World> toDraw);
		
		// Rml::SystemInterface overrides, used internally
		double GetElapsedTime() override;
		void SetMouseCursor(const Rml::String& cursor_name) override;
		void SetClipboardText(const Rml::String& text) override;
		void GetClipboardText(Rml::String& text) override;
		
		/// Called by RmlUi when it wants to render geometry that it does not wish to optimise.
		void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override;
		
		/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture) override;
		
		/// Called by RmlUi when it wants to render application-compiled geometry.
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) override;
		/// Called by RmlUi when it wants to release application-compiled geometry.
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override;
		
		/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
		void EnableScissorRegion(bool enable) override;
		/// Called by RmlUi when it wants to change the scissor region.
		void SetScissorRegion(int x, int y, int width, int height) override;
		
		/// Called by RmlUi when a texture is required by the library.
		bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
		/// Called by RmlUi when a loaded texture is no longer required.
		void ReleaseTexture(Rml::TextureHandle texture_handle) override;
		
		/// Called by RmlUi when it wants to set the current transform matrix to a new matrix.
		void SetTransform(const Rml::Matrix4f* transform) override;

#ifdef _DEBUG
		void InitDebugger() const;
		void DeactivateDebugger() const;
		static Ref<InputManager> debuggerInput;
#endif
						
    protected:
#ifdef _DEBUG
        struct DebugMsg{
            std::string message;
            uint8_t color;
        };
        static SpinLock dbgmtx;
        static phmap::flat_hash_map<uint16_t,DebugMsg> debugprints;
#endif
        void runAPIThread(bgfx::PlatformData pd, int width, int height);
		
		WeakRef<World> worldToDraw;
		std::optional<std::thread> renderThread;
		std::atomic<bool> render_thread_exit = false;
		std::atomic<bool> bgfx_thread_finished_init = false;
		ConcurrentQueue<std::function<void(void)>> RenderThreadQueue;
		float currentFrameTime;

		static SDL_Window* window;
        void Init();
		
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
		
		FrameData fd;
		
		/**
		 Execute instanced draw calls for a given light type
		 @param components the componetstore of the world to get the lights from
		 @return true light draw calls were executed, false otherwise
		 */
		template<typename LightType, class Container>
		inline bool DrawLightsOfType(const Container& lights){
			//must set before changing shaders
			if (lights.size() == 0){
				return false;
			}
			for(int i = 0; i < gbufferSize; i++){
				bgfx::setTexture(i, gBufferSamplers[i], attachments[i]);
			}
			
			const auto stride = LightType::InstancingStride();
			const auto numLights = lights.size();
			
			//create buffer for GPU instancing
			bgfx::InstanceDataBuffer idb;
			bgfx::allocInstanceDataBuffer(&idb, numLights, stride);
			
			//fill the buffer
			int i = 0;
			for(const auto& l : lights){	//TODO: factor in light frustum culling
				float* ptr = (float*)(idb.data + i);
				l.AddInstanceData(ptr);
				i += stride;
			}

			//fill the remaining slots in the buffer with 0s (if a light was removed during the buffer filling)
			for (; i < numLights; i++) {
				*(idb.data + i) = 0;
			}
			
			bgfx::setInstanceDataBuffer(&idb);
			
			//set the required state for this light type
			LightType::SetState();
			
			//execute instance draw call
			LightType::Draw(Views::Lighting);	//view 2 is the lighting pass
			
			return true;
		}
		
		static bgfx::VertexLayout RmlLayout;
		static Ref<GUIMaterialInstance> guiMaterial;
		
		matrix4 currentMatrix;
        
        struct scissor{
            uint16_t x, y, width, height;
            bool enabled = false;
        } RMLScissor;
		
#ifdef _DEBUG
		//used for the GUI debugger
		static Ref<Entity> debuggerContext;
#endif
	
    };
}
