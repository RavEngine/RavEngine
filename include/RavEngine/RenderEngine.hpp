#pragma once
//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Ref.hpp"
#include "WeakRef.hpp"
#include "Uniform.hpp"
#include <bgfx/bgfx.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <fmt/format.h>
#include "Uniform.hpp"
#include "TransientComputeBuffer.hpp"
#include <DebugDraw.h>
#include "Function.hpp"
#include "Common3D.hpp"
#include "SpinLock.hpp"
#include <RavEngine/Utilities.hpp>
#include "PhysXDefines.h"

struct SDL_Window;

namespace RavEngine {

	struct AppConfig;
    class DeferredBlitShader;
	class GUIMaterialInstance;
	class InputManager;
    struct Entity;
    struct World;

    class RenderEngine : public Rml::SystemInterface, public Rml::RenderInterface, public duDebugDraw {
	private:
		struct dim{
			int width, height;
		} bufferdims, windowdims;
		
		void UpdateBufferDims();
        
        uint32_t currentVRAM = 0;
        uint32_t totalVRAM = 0;
    public:
        virtual ~RenderEngine();
        RenderEngine(const AppConfig&);

		//render a world, for internal use only
		void Draw(Ref<World>);

		//get reset bitmask, for internal use only
		static uint32_t GetResetFlags();
        
        /**
         Print a string to the debug text overlay. Stubbed in release.
         Debug messages persist until they are overwritten or cleared.
         @param row the row to display the text
         */
        template<typename ... T>
        static void DebugPrint(uint16_t row, uint8_t color, const std::string& formatstr, T... args){
#ifdef _DEBUG
            dbgmtx.lock();
            debugprints[row] = {StrFormat(formatstr, args...),color};
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
        constexpr const dim& GetBufferSize(){
			return bufferdims;
		}
		
        constexpr const dim& GetWindowSize(){
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

//#ifdef _DEBUG
		void InitDebugger() const;
		void DeactivateDebugger() const;
		static Ref<InputManager> debuggerInput;
//#endif
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
        
    protected:
        static RavEngine::Vector<VertexColorUV> navMeshPolygon;
        static bgfx::VertexLayout debugNavMeshLayout;
        static bgfx::ProgramHandle debugNavProgram;
        bool navDebugDepthEnabled = false;
        
#ifdef _DEBUG
        struct DebugMsg{
            std::string message;
            uint8_t color;
        };
        static SpinLock dbgmtx;
        static UnorderedMap<uint16_t,DebugMsg> debugprints;
#endif

#ifdef _WIN32
		float win_scalefactor = 1;
#endif
		
		WeakRef<World> worldToDraw;
		std::optional<std::thread> renderThread;
		float currentFrameTime;

		static SDL_Window* window;
		void* metalLayer;
        void Init(const AppConfig&);
    public:
		static constexpr uint8_t gbufferSize = 4;
		static constexpr uint8_t lightingAttachmentsSize = 2;
    protected:
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

		TransientComputeBufferReadOnly skinningComputeBuffer;
		TransientComputeBuffer poseStorageBuffer;
					
        static bgfx::VertexBufferHandle screenSpaceQuadVert;
        static bgfx::IndexBufferHandle screenSpaceQuadInd;
		static bgfx::VertexBufferHandle opaquemtxhandle;


		static bgfx::ProgramHandle skinningShaderHandle;

		static bgfx::VertexLayout skinningOutputLayout, skinningInputLayout;
		
		bgfx::FrameBufferHandle createFrameBuffer(bool, bool);
		
        Vector4Uniform numRowsUniform, computeOffsetsUniform;
        std::optional<Vector4Uniform> timeUniform;
		
		static bgfx::VertexLayout RmlLayout;
		static Ref<GUIMaterialInstance> guiMaterial;
		
		matrix4 currentMatrix;
                
        struct scissor{
            uint16_t x, y, width, height;
            bool enabled = false;
        } RMLScissor;
		
#ifdef _DEBUG
		//used for the GUI debugger
		//static Entity debuggerContext;
        //static World debuggerWorld;
#endif
	
		static const std::string_view BackendStringName(bgfx::RendererType::Enum backend);
    };
}
