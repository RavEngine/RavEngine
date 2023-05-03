#pragma once
//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Ref.hpp"
#include "WeakRef.hpp"
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <fmt/format.h>
#include <DebugDraw.h>
#include "Function.hpp"
#include "Common3D.hpp"
#include "SpinLock.hpp"
#include "Utilities.hpp"
#include "Defines.hpp"
#include "PhysXDefines.h"
#include <RGL/RGL.hpp>
#include <RGL/TextureFormat.hpp>

struct SDL_Window;

namespace RavEngine {

	struct AppConfig;
    class DeferredBlitShader;
	class GUIMaterialInstance;
	class InputManager;
    struct Entity;
    class World;

    class RenderEngine : public Rml::SystemInterface, public Rml::RenderInterface, public duDebugDraw {
        friend class App;
	private:
		struct dim{
			int width, height;
		} bufferdims, windowdims;
		
		void UpdateBufferDims();
        
        uint32_t currentVRAM = 0;
        uint32_t totalVRAM = 0;

		RGLDevicePtr device;
		RGLFencePtr swapchainFence;
		RGLCommandQueuePtr mainCommandQueue;
		RGLCommandBufferPtr mainCommandBuffer;
		RGLSwapchainPtr swapchain;
		RGLSurfacePtr surface;

		RGLTexturePtr diffuseTexture, normalTexture, depthStencil, lightingTexture;
		RGLPipelineLayoutPtr lightRenderPipelineLayout, lightToFBPipelineLayout;
		RGLSamplerPtr textureSampler;
		RGLRenderPassPtr deferredRenderPass, lightingRenderPass, finalRenderPass;

		RGLRenderPipelinePtr ambientLightRenderPipeline, dirLightRenderPipeline, lightToFBRenderPipeline;
		RGLBufferPtr screenTriVerts;
    public:
		constexpr static RGL::TextureFormat
			normalTexFormat = RGL::TextureFormat::RGBA16_Sfloat,
			colorTexFormat = RGL::TextureFormat::RGBA16_Snorm;

		// the items made available to 
		// user-defined materials
		struct DeferredUBO {
			glm::mat4 viewProj;
		};

		struct LightingUBO {
			glm::ivec4 viewRect;
			uint32_t numObjects;
		};

        virtual ~RenderEngine();
        RenderEngine(const AppConfig&);

		void createGBuffers();

		//render a world, for internal use only
		void Draw(Ref<RavEngine::World>);
        
        /**
         @return The name of the current rendering API in use
         */
        static const std::string_view GetCurrentBackendName();

		static SDL_Window* const GetWindow(){
			return window;
		}
        
        enum class WindowMode{
            Windowed,
            BorderlessFullscreen,
            Fullscreen
        };
        /**
         Set the video mode
         @param mode the new mode to use
         */
        void SetWindowMode(WindowMode mode);

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
        constexpr const dim& GetBufferSize() const{
			return bufferdims;
		}
		
        constexpr const dim& GetWindowSize() const{
			return windowdims;
		}
		
        /**
         @return the High DPI scale factor. Only applicable on macOS.
         */
        constexpr float GetDPIScale() const{
#ifndef _WIN32
			return (float)bufferdims.width / windowdims.width;
#else
			return win_scalefactor;
#endif
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

#ifndef NDEBUG
		void InitDebugger() const;
		void DeactivateDebugger() const;
		static std::unique_ptr<InputManager> debuggerInput;
#endif
        // navigation debug rendering -- internal use only
        void depthMask(bool state) final;
        void texture(bool state) final;
        void begin(duDebugDrawPrimitives prim, float size = 1.0f) final;
        void vertex(const float* pos, unsigned int color) final;
        void vertex(const float x, const float y, const float z, unsigned int color) final;
        void vertex(const float* pos, unsigned int color, const float* uv) final;
        void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) final;
        void end() final;
        
        decltype(currentVRAM) GetCurrentVRAMUse(){
            return currentVRAM;
        }
        
        decltype(totalVRAM) GetTotalVRAM(){
            return totalVRAM;
        }

		// API for interacting with the GPU
		auto GetDevice() {
			return device;
		}
        
		ConcurrentQueue<RGLBufferPtr> gcBuffers;
		ConcurrentQueue<RGLTexturePtr> gcTextures;
		ConcurrentQueue<RGLPipelineLayoutPtr> gcPipelineLayout;
		ConcurrentQueue<RGLRenderPipelinePtr> gcRenderPipeline;


    protected:
		void DestroyUnusedResources();
        static RavEngine::Vector<VertexColorUV> navMeshPolygon;
        bool navDebugDepthEnabled = false; 

#ifdef _WIN32
		float win_scalefactor = 1;
#endif
		
		float currentFrameTime;

		static SDL_Window* window;
		void* metalLayer;
        void Init(const AppConfig&);
    public:

    protected:
		
        static Ref<GUIMaterialInstance> guiMaterial;
				
		matrix4 currentGUIMatrix;
                
        struct scissor{
            uint16_t x, y, width, height;
            bool enabled = false;
        } RMLScissor;
	
    };
}
