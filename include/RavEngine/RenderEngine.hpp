#pragma once
//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//
#if !RVE_SERVER
#include "Ref.hpp"
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
#include "Mesh.hpp"

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

    class RenderEngine : public Rml::RenderInterface, public duDebugDraw {
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
		RGLSamplerPtr textureSampler, textureClampSampler, shadowSampler, depthPyramidSampler;
		RGLRenderPassPtr litRenderPass, unlitRenderPass, envSkyboxPass, depthPrepassRenderPass, depthPrepassRenderPassLit, postProcessRenderPass, postProcessRenderPassClear, finalRenderPass, finalRenderPassNoDepth, shadowRenderPass, lightingClearRenderPass, litClearRenderPass, finalClearRenderPass, depthPyramidCopyPass, litTransparentPass, unlitTransparentPass, transparentClearPass, transparencyApplyPass, ssaoPassClear, ssgiPassNoClear, ssgiAmbientApplyPass, ssgiPassClear;

		RGLRenderPipelinePtr depthPyramidCopyPipeline,
			im3dLineRenderPipeline, im3dPointRenderPipeline, im3dTriangleRenderPipeline, recastLinePipeline, recastPointPipeline, recastTrianglePipeline, guiRenderPipeline, transparencyApplyPipeline, ssgipipeline, gtaoPipeline, ssaoPipeline, ssgiDownsamplePipeline, ssgiUpsamplePipeline, ambientSSGIApplyPipeline, aoUpsamplePipeline, ssgiUpsamplePipleineFinalStep, environmentPreFilterPipeline, environmentIrradiancePipeline;
		RGLComputePipelinePtr skinnedMeshComputePipeline, defaultCullingComputePipeline, skinningDrawCallPreparePipeline, depthPyramidPipeline, particleCreatePipeline, particleDispatchSetupPipeline, particleDispatchSetupPipelineIndexed, particleKillPipeline, clusterBuildGridPipeline, clusterPopulatePipeline, blurKernelX, blurKernelY;
		RGLBufferPtr screenTriVerts,
			sharedPositionBuffer,sharedNormalBuffer, sharedTangentBuffer, sharedBitangentBuffer, sharedUV0Buffer, sharedLightmapUVBuffer, sharedIndexBuffer, sharedSkeletonMatrixBuffer, sharedSkinnedPositionBuffer, sharedSkinnedNormalBuffer, sharedSkinnedTangentBuffer, sharedSkinnedBitangentBuffer, sharedSkinnedUV0Buffer, quadVertBuffer, lightClusterBuffer, debugRenderBufferUpload;
		uint32_t debugRenderBufferSize = 0, debugRenderBufferOffset = 0;

		constexpr static uint32_t initialVerts = 1024, initialIndices = 1536;

		struct PyramidCopyUBO {
			uint32_t size;
		};

		Ref<DummyTonemapInstance> dummyTonemap;
        
        enum DefaultCullBindings : uint32_t{
            Cubo = 0,
            modelMatrix = 1,
            renderLayer = 2,
            perObject = 7,
            depthPyramid = 8,
            depthPyramidSamplerBinding = 9,
        };

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

		struct SkyboxEnvPrefilterUBO {
			float roughness;
		};

		struct AmbientSSGIApplyUBO {
			glm::mat4 invView{ 1 };
			uint32_t ambientLightCount = 0;
			float ssaoStrength = 1;
			uint32_t options = 0;
			static constexpr uint32_t SSAOBIT = 1, SSGIBIT = 1 << 1;
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
			ssaoOutputFormat = RGL::TextureFormat::R16_Float,
			depthFormat = RGL::TextureFormat::D32SFloat;

		struct navDebugUBO {
			glm::mat4 model;
			glm::mat4 viewProj;
		} currentNavState;

		struct LightToFBUBO {
			glm::ivec4 viewRect;
		};

		struct GUIUBO {
			glm::mat4 transform, projection;
            glm::vec2 translate;
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

		struct GTAOUBO {
			glm::ivec2 screenDim;
		};

		struct SSAOUBO {
			glm::mat4 proj;
			glm::mat4 invProj;
			glm::ivec2 screenDim;
		};

		struct JointInfluence {
			uint32_t joint_index;
			float influence;
		};

		struct CullingUBOinstance {
			uint32_t indirectBufferOffset = 0;	// needs to be like this because of padding / alignment
			uint32_t numObjects = 0;
			uint32_t cullingBufferOffset = 0;
            float radius = 0;
			uint32_t numLODs = 0;
			uint32_t idOutputBufferBindlessHandle = 0;
			uint32_t entityIDInputBufferBindlessHandle = 0;
			uint32_t indirectOutputBufferBindlessHandle = 0;
			uint32_t lodDistanceBufferBindlessHandle = 0;
		};

		struct CullingUBO {
			glm::mat4 viewProj;
			glm::vec3 camPos;
			uint32_t numCubos;
			renderlayer_t cameraRenderLayers = 0;
			uint32_t singleInstanceModeAndShadowMode = 0;	// skinning vs not skinning
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

		enum class AOMethod : uint8_t {
			SSAO,
			GTAO
		};
		
		static struct vs {
			bool vsync = true;
			AOMethod aoMethod = AOMethod::SSAO;
		} VideoSettings;

		void SyncVideoSettings();
        
		
		// Rml::RenderInterface overrides, used internally
		Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) final;
		void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) final;
		void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) final;
		void EnableScissorRegion(bool enable) final;
		void SetScissorRegion(Rml::Rectanglei region) final;
		Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) final;
		Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) final;
		void ReleaseTexture(Rml::TextureHandle texture_handle) final;
		void SetTransform(const Rml::Matrix4f* transform) final;

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

		MeshRange AllocateMesh(const MeshPartView& mesh);

		void DeallocateMesh(const MeshRange& range);

    protected:
		dim_t<int> currentRenderSize;
		matrix4 make_gui_matrix(Rml::Vector2f translation);

		constexpr static uint32_t transientSizeBytes = 32 * 1024 * 1024;
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
				
		matrix4 currentGUIMatrix = matrix4(1);
                
        struct scissor{
            uint16_t x, y, width, height;
            bool enabled = false;
        } RMLScissor;
	
    };
}
#endif
