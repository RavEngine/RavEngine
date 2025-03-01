#pragma once
#include <RGL/Types.hpp>
#include "mathtypes.hpp"
#include <string>
#include <variant>
#include "Ref.hpp"
#include <RGL/Span.hpp>
#include <RGL/Pipeline.hpp>
#include <bitset>
#include <span>
#include "MaterialShared.hpp"

namespace RavEngine {

	extern const RGL::RenderPipelineDescriptor::VertexConfig quadVertexConfig;
	extern const RGL::RenderPipelineDescriptor::VertexConfig defaultVertexConfig;

	struct Texture;
	struct MeshCollectionStatic;

	struct ParticleUpdateUBO {
		float fpsScale;
	};

	struct ParticleBillboardUBO {
		vector2i spritesheetDim;
		vector2i numSprites;
		uint32_t bytesPerParticle;
		uint32_t particlePositionOffset;
		uint32_t particleScaleOffset;
		uint32_t particleFrameOffset;
	};

	struct ParticleQuadVert {
		vector2 pos;
	};

	struct ParticleUpdateMaterial {
		friend class RenderEngine;
	protected:
		ParticleUpdateMaterial(const std::string_view initShaderName, const std::string_view updateShaderName);
		
	public:
		const auto GetInitShader() const {
			return userInitPipeline;
		}

		const auto GetUpdateShader() const {
			return userUpdatePipeline;
		}

	private:
		RGLComputePipelinePtr userInitPipeline, userUpdatePipeline;
	};

	struct ParticleRenderMaterialConfig {
		std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> bindings;
		size_t pushConstantSize = 0;
		OpacityMode opacityMode = OpacityMode::Opaque;
		bool zTestEnabled = true;
		bool zWriteEnabled = true;
	}; 

	enum class LightingMode : uint8_t {
		Unlit,
		Lit
	};

	struct ParticleRenderMaterial {
		friend class RenderEngine;
		constexpr static uint8_t
			particleDataBufferBinding = 18,
			particleAliveIndexBufferBinding = 19,
			particleMatrixBufferBinding = 20,
            particleEmitterStateBufferBinding = 22
			;

		bool IsTransparent() const {
			return opacityMode == OpacityMode::Transparent;
		}
	protected:
		struct InternalConfig {
			const RGL::RenderPipelineDescriptor::VertexConfig vertexConfig;
			const RGL::PrimitiveTopology topology;
			const LightingMode mode;
		};
		ParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const InternalConfig& internalConfig, const ParticleRenderMaterialConfig& config);

		RGLRenderPipelinePtr GetMainRenderPipeline() const {
			return userRenderPipeline;
		}

		RGLRenderPipelinePtr GetShadowRenderPipeline() const {
			return shadowRenderPipeline;
		}

		auto GetDepthPrepassPipeline() const {
			return prepassRenderPipeline;
		}

	private:
		OpacityMode opacityMode = OpacityMode::Opaque;
		RGLRenderPipelinePtr userRenderPipeline, prepassRenderPipeline, shadowRenderPipeline;

		
	};

	struct BillboardRenderParticleMaterial_impl {
		ParticleRenderMaterialConfig augmentBillboardConfig(const ParticleRenderMaterialConfig& in, LightingMode mode);
	};

	// Subclass this to make custom particle materials
	template<LightingMode mode>
	struct BillboardRenderParticleMaterial : public ParticleRenderMaterial, private BillboardRenderParticleMaterial_impl {
	protected:
		BillboardRenderParticleMaterial(const std::string_view particleVS, const std::string_view particleFS, const ParticleRenderMaterialConfig& config = {}) : ParticleRenderMaterial(particleVS, particleFS, {
			.vertexConfig = quadVertexConfig,
			.topology = RGL::PrimitiveTopology::TriangleStrip,
			.mode = mode
			}, augmentBillboardConfig(config,mode)) {}
	};

	struct MeshParticleRenderMaterial_Impl {
		ParticleRenderMaterialConfig augmentMeshConfig(const ParticleRenderMaterialConfig& in, LightingMode mode);
	};

	// Subclass this to make custom particle materials
	template<LightingMode mode>
	struct MeshParticleRenderMaterial : public ParticleRenderMaterial, private MeshParticleRenderMaterial_Impl {
	protected:
		MeshParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const ParticleRenderMaterialConfig& config = {}) : ParticleRenderMaterial(particleVS, particleFS, {
		.vertexConfig = defaultVertexConfig,
		.topology = RGL::PrimitiveTopology::TriangleList,
		.mode = mode
			}, augmentMeshConfig(config, mode)) {}
	};

	// Subclass to create custom mesh particle selection 
	struct MeshParticleMeshSelectionMaterial {
		friend class RenderEngine;
		MeshParticleMeshSelectionMaterial(const std::string_view name);
	private:
		RGLComputePipelinePtr userSelectionPipeline;
	};

	// subclass this in tandem with the SelectionMaterial
	struct MeshParticleMeshSelectionMaterialInstance {
		friend class RenderEngine;
		MeshParticleMeshSelectionMaterialInstance(Ref<MeshParticleMeshSelectionMaterial> mat) : material(mat) {}
	private:
		Ref<MeshParticleMeshSelectionMaterial> material = nullptr;
	};

	struct ParticleRenderMaterialInstance {
		friend class RenderEngine;
	protected:
		std::array<Ref<Texture>, 16> textureBindings;
		std::bitset<16> samplerBindings;
	public:

		
		/**
		Provide data to be uploaded to shader push constants.
		@return Number of bytes written, 0 if there is no data to upload.
		*/
		virtual uint8_t SetPushConstantData(std::span<std::byte,128> data) const {
			return 0;
		}
	};

	// subclass to define your own particle material instances
	struct BillboardParticleRenderMaterialInstance : ParticleRenderMaterialInstance {
		using MaterialVariant = std::variant<Ref<BillboardRenderParticleMaterial<LightingMode::Lit>>, Ref<BillboardRenderParticleMaterial<LightingMode::Unlit>> >;
	protected:
		MaterialVariant material;
	public:

		BillboardParticleRenderMaterialInstance(const decltype(material)& mat) : material(mat) {}
		
		auto GetMaterial() const {
			return material;
		}
	};

	// subclass to define your own particle material instances
	struct MeshParticleRenderMaterialInstance : ParticleRenderMaterialInstance {
		friend class RenderEngine;
		static constexpr uint32_t kEngineDataBinding = 21;
		using MaterialVariant = std::variant<Ref<MeshParticleRenderMaterial<LightingMode::Lit>>, Ref<MeshParticleRenderMaterial<LightingMode::Unlit>> >;
	private:
		MaterialVariant material;
	public:
		// use this constructor if you want to use the default mesh selection behavior (all particles go to mesh 0)
		MeshParticleRenderMaterialInstance(const decltype(material)& mat, Ref<MeshCollectionStatic> meshes) : material(mat), meshes(meshes) {}

		// use this constructor if you have custom mesh selection logic to apply
		MeshParticleRenderMaterialInstance(const decltype(material)& mat, Ref<MeshCollectionStatic> meshes, Ref<MeshParticleMeshSelectionMaterialInstance> customSelection) : material(mat), meshes(meshes), customSelectionFunction(customSelection) {}

		void SetMeshSelectionFunction(const Ref<MeshParticleMeshSelectionMaterialInstance> selfn) {
			customSelectionFunction = selfn;
		}

		auto GetMaterial() const {
			return material;
		}
	private:
		Ref<MeshCollectionStatic> meshes;
		Ref<MeshParticleMeshSelectionMaterialInstance> customSelectionFunction = nullptr;
	};

	// subclass to define your own particle material instances
	struct ParticleUpdateMaterialInstance {
		Ref<ParticleUpdateMaterial> mat;
		ParticleUpdateMaterialInstance(decltype(mat) m) : mat(m) {}
	};

	// built-in billboard renderer that does spritesheets
	template<LightingMode mode>
	struct SpritesheetParticleRenderMaterial : public RavEngine::BillboardRenderParticleMaterial<mode> {
		constexpr static uint16_t SpritesheetBindingSlot = 1;
		constexpr static uint16_t SamplerBindingSlot = 0;



		SpritesheetParticleRenderMaterial(const std::string_view vs_name, const std::string_view fs_name, OpacityMode opacityMode, size_t pushConstantSize) : BillboardRenderParticleMaterial<mode>(vs_name, fs_name, {
			.bindings = {
				{
					.binding = SamplerBindingSlot,
					.type = RGL::BindingType::Sampler,
					.stageFlags = RGL::BindingVisibility::Fragment,
					.writable = false,
				},
				{
					.binding = SpritesheetBindingSlot,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
					.writable = false,
				}
			},
			.pushConstantSize = pushConstantSize,
			.opacityMode = opacityMode
			}) {}

		constexpr static std::string_view fs_particle_name = mode == LightingMode::Lit ? "default_billboard_particle" : "default_billboard_particle_unlit";
		SpritesheetParticleRenderMaterial() : SpritesheetParticleRenderMaterial("default_billboard_particle", fs_particle_name, OpacityMode::Opaque, sizeof(ParticleBillboardUBO)) {}
	};

	struct SpritesheetParticleRenderMaterialInstance : BillboardParticleRenderMaterialInstance {
		
		template<LightingMode mode>
		SpritesheetParticleRenderMaterialInstance(Ref<SpritesheetParticleRenderMaterial<mode>> mat, uint32_t bytesPerParticle, uint32_t particlePositionOffset, uint32_t particleScaleOffset, uint32_t particleFrameOffset) : BillboardParticleRenderMaterialInstance(mat), bytesPerParticle(bytesPerParticle), particlePositionOffset(particlePositionOffset), particleScaleOffset(particleScaleOffset), particleFrameOffset(particleFrameOffset){
			samplerBindings[SpritesheetParticleRenderMaterial<LightingMode::Lit>::SamplerBindingSlot] = true;
		};
		struct spriteCount {
			uint16_t numSpritesWidth = 0;
			uint16_t numSpritesHeight = 0;
		} spriteDim;

		void SetSpritesheet(Ref<Texture> spriteTex) {
			textureBindings[SpritesheetParticleRenderMaterial<LightingMode::Lit>::SpritesheetBindingSlot] = spriteTex;
		}

		virtual uint8_t SetPushConstantData(std::span<std::byte, 128> data) const;
	private:
		const uint32_t bytesPerParticle;
		const uint32_t particlePositionOffset;
		const uint32_t particleScaleOffset;
		const uint32_t particleFrameOffset;
	};

	struct PBRMeshParticleRenderMaterial : public MeshParticleRenderMaterial<LightingMode::Lit> {
		PBRMeshParticleRenderMaterial();
	};

	struct PBRMeshParticleRenderMaterialInstance : public MeshParticleRenderMaterialInstance {
		PBRMeshParticleRenderMaterialInstance(Ref<PBRMeshParticleRenderMaterial> mat, Ref<MeshCollectionStatic> meshes, uint32_t bytesPerParticle, uint32_t positionOffsetBytes, uint32_t scaleOffsetBytes, uint32_t rotationOffsetBytes);
		virtual uint8_t SetPushConstantData(std::span<std::byte, 128> data) const;

		constexpr static uint8_t
			kSamplerBinding = 0,
			kDiffuseBinding = 1,
			kNormalBinding = 2,
			kSpecularBinding = 3,
			kMetallicBinding = 4,
			kRoughnessBinding = 5,
			kAOBinding = 6,
			kEmissiveBinding = 7
			;

	private:
		const uint32_t bytesPerParticle;
		const uint32_t positionOffsetBytes;
		const uint32_t scaleOffsetBytes;
		const uint32_t rotationOffsetBytes;
	};
}
