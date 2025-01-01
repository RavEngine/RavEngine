#pragma once
//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//
#if !RVE_SERVER
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
#include "RenderTargetCollection.hpp"
#include "PostProcess.hpp"
#include <unordered_set>
#include "cluster_defs.h"
#include "Queue.hpp"
#include "Layer.hpp"

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
	struct DummyTonemapInstance;

	namespace Clustered {
		constexpr static uint32_t gridSizeX = 12;
		constexpr static uint32_t gridSizeY = 12;
		constexpr static uint32_t gridSizeZ = 24;
		constexpr static uint32_t numClusters = gridSizeX * gridSizeY * gridSizeZ;

		struct Cluster {
			glm::vec3 minPoint;
			glm::vec3 maxPoint;
			uint32_t pointLightCount;
			uint32_t spotLightCount;
			uint32_t pointLightIndices[CLUSTER_MAX_POINTS];
			uint32_t spotLightIndices[CLUSTER_MAX_SPOTS];
		};
	}

    class RenderEngine : public Rml::SystemInterface, public Rml::RenderInterface, public duDebugDraw {
        friend class App;
	private:
		uint64_t frameCount = 0;
		decltype(frameCount) lastResizeFrameVB = 0, lastResizeFrameIB = 0;

		using dim = dim_t<uint32_t>;

		RGLDevicePtr device;
		RGLCommandQueuePtr mainCommandQueue;
		RGLCommandBufferPtr mainCommandBuffer, transformSyncCommandBuffer, transientCommandBuffer;
		bool transientSubmittedLastFrame = false;

		RGLTexturePtr dummyShadowmap, dummyCubemap;
		RGLSamplerPtr textureSampler, shadowSampler, depthPyramidSampler;
		RGLRenderPassPtr litRenderPass, unlitRenderPass, depthPrepassRenderPass, postProcessRenderPass, postProcessRenderPassClear, finalRenderPass, shadowRenderPass, lightingClearRenderPass, litClearRenderPass, finalClearRenderPass, depthPyramidCopyPass, litTransparentPass, unlitTransparentPass, transparentClearPass, transparencyApplyPass, ssgiPassNoClear, ssgiAmbientApplyPass, ssgiPassClear;

		RGLRenderPipelinePtr depthPyramidCopyPipeline,
			im3dLineRenderPipeline, im3dPointRenderPipeline, im3dTriangleRenderPipeline, recastLinePipeline, recastPointPipeline, recastTrianglePipeline, guiRenderPipeline, transparencyApplyPipeline, ssgipipeline, ssgiDownsamplePipeline, ssgiUpsamplePipeline, ambientSSGIApplyPipeline, aoUpsamplePipeline, ssgiUpsamplePipleineFinalStep;
		RGLComputePipelinePtr skinnedMeshComputePipeline, defaultCullingComputePipeline, skinningDrawCallPreparePipeline, depthPyramidPipeline, particleCreatePipeline, particleDispatchSetupPipeline, particleDispatchSetupPipelineIndexed, particleKillPipeline, clusterBuildGridPipeline, clusterPopulatePipeline;
		RGLBufferPtr screenTriVerts,
			sharedVertexBuffer, sharedIndexBuffer, sharedSkeletonMatrixBuffer, sharedSkinnedMeshVertexBuffer, quadVertBuffer, lightClusterBuffer, debugRenderBufferUpload;
		uint32_t debugRenderBufferSize = 0, debugRenderBufferOffset = 0;

		constexpr static uint32_t initialVerts = 1024, initialIndices = 1536;

		struct PyramidCopyUBO {
			uint32_t size;
		};

		Ref<DummyTonemapInstance> dummyTonemap;

#pragma pack(push, 1)
		struct GridBuildUBO {
			glm::mat4 invProj{ 0 };
			glm::uvec3 gridSize{ 0 };
			float zNear = 0;
			glm::uvec2 screenDim{ 0 };
			float zFar = 0;
		};
#pragma pack(pop)

		struct GridAssignUBO {
			glm::mat4 viewMat;
			uint32_t pointLightCount, spotLightCount;
		};

		struct DownsampleUBO {
			glm::uvec4 targetDim;
		};

		struct UpsampleUBO {
			glm::uvec4 targetDim;
			float filterRadius;
		};

		struct AmbientSSGIApplyUBO {
			uint32_t ambientLightCount = 0;
		};
        
        RGLShaderLibraryPtr defaultPostEffectVSH;

		friend class Material;
    public:
		static constexpr RGL::TextureFormat depthPyramidFormat = RGL::TextureFormat::R32_Float;
		constexpr static uint32_t maxssgimips = 5;

        auto GetDefaultPostEffectVSH() const{
            return defaultPostEffectVSH;
        }
                
		constexpr static RGL::TextureFormat
			colorTexFormat = RGL::TextureFormat::RGBA16_Sfloat,
			radianceFormat = RGL::TextureFormat::RGBA16_Sfloat,
			albedoFormat = RGL::TextureFormat::RGBA16_Sfloat,
			viewSpaceNormalTextureFormat = RGL::TextureFormat::RGBA16_Sfloat,
			ssgiOutputFormat = RGL::TextureFormat::RGBA16_Sfloat,
			depthFormat = RGL::TextureFormat::D32SFloat;

		struct navDebugUBO {
			glm::mat4 model;
			glm::mat4 viewProj;
		} currentNavState;

		struct LightToFBUBO {
			glm::ivec4 viewRect;
		};

		struct GUIUBO {
			glm::mat4 viewProj;
		};

		struct DebugUBO {
			glm::mat4 viewProj;
		};

		struct SkinningUBO {
			uint32_t numObjects = 0;
			uint32_t numVertices = 0;
			uint32_t numBones = 0;
			uint32_t boneReadOffset = 0;
			uint32_t vertexWriteOffset = 0;
			uint32_t vertexReadOffset = 0;
		};

		struct SSGIUBO {
			glm::mat4 projection;
			glm::mat4 invProj;
			glm::ivec2 outputDim;
			float sampleCount;
			float sampleRadius;
			float sliceCount;
			float hitThickness;
		};

		struct JointInfluence {
			uint32_t joint_index;
			float influence;
		};

		struct CullingUBO {
			glm::mat4 viewProj;
			glm::vec3 camPos;
			uint32_t indirectBufferOffset = 0;	// needs to be like this because of padding / alignment
			uint32_t numObjects = 0;
			uint32_t cullingBufferOffset = 0;
            float radius = 0;
			uint32_t singleInstanceModeAndShadowMode = 0;	// skinning vs not skinning
			uint32_t numLODs = 0;
            renderlayer_t cameraRenderLayers = 0;
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

		struct ParticleCreationPushConstants {
			uint32_t particlesToSpawn;
			uint32_t maxParticles;
		};

		struct KillParticleUBO {
			uint32_t maxTotalParticles;
		};

		virtual ~RenderEngine();
        RenderEngine(const AppConfig&, RGLDevicePtr device);

		RenderTargetCollection CreateRenderTargetCollection(dim size, bool createDepth = true);
		void ResizeRenderTargetCollection(RenderTargetCollection& collection, dim size);

		//render a world, for internal use only
		RGLCommandBufferPtr Draw(Ref<RavEngine::World>, const std::span<RenderViewCollection> screenTargets, float guiScaleFactor);
        
        /**
         @return The name of the current rendering API in use
         */
        static const std::string_view GetCurrentBackendName();
		
		static struct vs {
			bool vsync = true;
		} VideoSettings;

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
        
		size_t GetCurrentVRAMUse();
        
		size_t GetTotalVRAM();

		ConcurrentQueue<RGLBufferPtr> gcBuffers;
		ConcurrentQueue<RGLTexturePtr> gcTextures;
		ConcurrentQueue<RGLPipelineLayoutPtr> gcPipelineLayout;
		ConcurrentQueue<RGLRenderPipelinePtr> gcRenderPipeline;

		MeshRange AllocateMesh(const std::span<const VertexNormalUV> vertices, const std::span<const uint32_t> index_bytes);

		void DeallocateMesh(const MeshRange& range);

    protected:
		dim_t<int> currentRenderSize;
		matrix4 make_gui_matrix(Rml::Vector2f translation);

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
		void ReallocateGeneric(RGLBufferPtr& reallocBuffer, uint32_t& reallocBufferSize, uint32_t newSize, allocation_allocatedlist_t& allocatedList, allocation_freelist_t& freelist, uint32_t stride, RGL::BufferConfig::Type bufferType, decltype(frameCount)& lastResizeFrame, const char* debugName = nullptr);

		SpinLock allocationLock;

		void DestroyUnusedResources();
        static RavEngine::Vector<VertexColorUV> navMeshPolygon;
        bool navDebugDepthEnabled = false; 

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
#endif
