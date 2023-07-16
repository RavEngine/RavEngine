#pragma once
//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Ref.hpp"
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <DebugDraw.h>
#include "Common3D.hpp"
#include "Defines.hpp"
#include "PhysXDefines.h"
#include <RGL/Types.hpp>
#include <RGL/TextureFormat.hpp>
#include <RGL/Buffer.hpp>
#include <span>
#include "SpinLock.hpp"
#include "MeshAllocation.hpp"
#include <unordered_set>

struct SDL_Window;

namespace Im3d {
	struct DrawList;
};

namespace RavEngine {

	struct AppConfig;
    class DeferredBlitShader;
	class GUIMaterialInstance;
	class InputManager;
    struct Entity;
    class World;
	struct MeshAsset;
	struct GUIComponent;

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

		RGLTexturePtr diffuseTexture, normalTexture, depthStencil, lightingTexture, shadowTexture;
		RGLPipelineLayoutPtr ambientLightRenderPipelineLayout, lightRenderPipelineLayout, lightToFBPipelineLayout, pointLightRenderPipelineLayout;
		RGLSamplerPtr textureSampler, shadowSampler;
		RGLRenderPassPtr deferredRenderPass, lightingRenderPass, finalRenderPass, shadowRenderPass;

		RGLRenderPipelinePtr ambientLightRenderPipeline, dirLightRenderPipeline, pointLightRenderPipeline, spotLightRenderPipeline, lightToFBRenderPipeline,
			im3dLineRenderPipeline, im3dPointRenderPipeline, im3dTriangleRenderPipeline, guiRenderPipeline;
		RGLComputePipelinePtr skinnedMeshComputePipeline, defaultCullingComputePipeline, skinningDrawCallPreparePipeline;
		RGLBufferPtr screenTriVerts, pointLightVertexBuffer, pointLightIndexBuffer, spotLightVertexBuffer, spotLightIndexBuffer,
			sharedVertexBuffer, sharedIndexBuffer, sharedSkeletonMatrixBuffer, sharedSkinnedMeshVertexBuffer;
		uint32_t nPointLightIndices = 0, nSpotLightIndices = 0;

		constexpr static uint32_t initialVerts = 1024, initialIndices = 1536;

		friend class Material;
    public:
		constexpr static RGL::TextureFormat
			normalTexFormat = RGL::TextureFormat::RGBA16_Sfloat,
			colorTexFormat = RGL::TextureFormat::RGBA16_Snorm;

		// the items made available to 
		// user-defined materials
		struct DeferredUBO {
			glm::mat4 viewProj;
		};

		struct AmbientLightUBO {
			glm::ivec4 viewRect;
		};

		struct LightingUBO {
			glm::mat4 viewProj;
			glm::ivec4 viewRect;
		};

		struct PointLightUBO {
			glm::mat4 viewProj;
			glm::mat4 invViewProj;
			glm::ivec4 viewRect;
		};

		struct SkinningUBO {
			uint32_t numObjects = 0;
			uint32_t numVertices = 0;
			uint32_t numBones = 0;
			uint32_t boneReadOffset = 0;
			uint32_t vertexWriteOffset = 0;
			uint32_t vertexReadOffset = 0;
		};

		struct JointInfluence {
			uint32_t joint_index;
			float influence;
		};

		struct alignas(16) CullingUBO {
			glm::mat4 viewProj;
			uint32_t indirectBufferOffset = 0;
			uint32_t numObjects = 0;
			uint32_t cullingBufferOffset = 0;
		};

		struct SkinningPrepareUBO {
			uint32_t indexBufferOffset = 0;
			uint32_t vertexBufferOffset = 0;
			uint32_t nIndicesInThisMesh = 0;
			uint32_t nVerticesInThisMesh = 0;
			uint32_t nTotalObjects = 0;
			uint32_t drawCallBufferOffset = 0;
			uint32_t baseInstanceOffset = 0;
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
         @return the time in microseconds to render the last frame (begin-to-present)
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
		static std::optional<GUIComponent> debuggerContext;
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

		void DebugRender(const Im3d::DrawList&);
        
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


		MeshRange AllocateMesh(const std::span<const VertexNormalUV> vertices, const std::span<const uint32_t> index_bytes);

		void DeallocateMesh(const MeshRange& range);

    protected:
	
		constexpr static uint32_t transientSizeBytes = 65536;
		RGLBufferPtr transientBuffer, transientStagingBuffer;
		uint32_t transientOffset = 0;
		
		/**
		* Add data to the transient buffer.
		* The transient buffer is reset every frame. 
		* @return the offset in bytes to the transient buffer
		*/
		uint32_t WriteTransient(RGL::untyped_span data);

		allocation_freelist_t vertexFreeList{ 1, Range{.start = 0, .count = initialVerts}};
		allocation_freelist_t indexFreeList{ 1, Range{.start = 0, .count = initialIndices}};
		allocation_allocatedlist_t vertexAllocatedList, indexAllocatedList;
		uint32_t currentVertexSize = initialVerts, currentIndexSize = initialIndices;
		
		void ReallocateVertexAllocationToSize(uint32_t newSize);
		void ReallocateIndexAllocationToSize(uint32_t newSize);
		void ReallocateGeneric(RGLBufferPtr& reallocBuffer, uint32_t& reallocBufferSize, uint32_t newSize, allocation_allocatedlist_t& allocatedList, allocation_freelist_t& freelist, uint32_t stride, RGL::BufferConfig::Type bufferType);

		SpinLock allocationLock;

		void DestroyUnusedResources();
        static RavEngine::Vector<VertexColorUV> navMeshPolygon;
        bool navDebugDepthEnabled = false; 

#ifdef _WIN32
		float win_scalefactor = 1;
#endif
		
		uint32_t currentFrameTime;

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
