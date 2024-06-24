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

namespace RavEngine {

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
	}; 

	struct ParticleRenderMaterial {
		friend class RenderEngine;
		constexpr static uint8_t
			particleDataBufferBinding = 12,
			particleAliveIndexBufferBinding = 13,
			particleMatrixBufferBinding = 14
			;
	protected:
		struct InternalConfig {
			RGL::RenderPipelineDescriptor::VertexConfig vertexConfig;
		};
		ParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const InternalConfig& internalConfig, const ParticleRenderMaterialConfig& config);


	private:
	
		RGLRenderPipelinePtr userRenderPipeline;
	};

	// Subclass this to make custom particle materials
	struct BillboardRenderParticleMaterial : public ParticleRenderMaterial {
	protected:
		BillboardRenderParticleMaterial(const std::string_view particleVS, const std::string_view particleFS, const ParticleRenderMaterialConfig& config = {});
	};

	// Subclass this to make custom particle materials
	struct MeshParticleRenderMaterial : public ParticleRenderMaterial {
	protected:
		MeshParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const ParticleRenderMaterialConfig& config = {});
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
		Ref<ParticleRenderMaterial> material;
		std::array<Ref<Texture>, 16> textureBindings;
		std::bitset<16> samplerBindings;
	public:
		ParticleRenderMaterialInstance(decltype(material) m) : material(m) {};

		auto GetMaterial() const {
			return material;
		}

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
		BillboardParticleRenderMaterialInstance(Ref<BillboardRenderParticleMaterial> mat) : ParticleRenderMaterialInstance(mat) {}
		
	};

	// subclass to define your own particle material instances
	struct MeshParticleRenderMaterialInstance : ParticleRenderMaterialInstance {
		friend class RenderEngine;

		// use this constructor if you want to use the default mesh selection behavior (all particles go to mesh 0)
		MeshParticleRenderMaterialInstance(Ref<MeshParticleRenderMaterial> mat, Ref<MeshCollectionStatic> meshes) : ParticleRenderMaterialInstance(mat), meshes(meshes) {}

		// use this constructor if you have custom mesh selection logic to apply
		MeshParticleRenderMaterialInstance(Ref<MeshParticleRenderMaterial> mat, Ref<MeshCollectionStatic> meshes, Ref<MeshParticleMeshSelectionMaterialInstance> customSelection) : ParticleRenderMaterialInstance(mat), meshes(meshes), customSelectionFunction(customSelection) {}

		void SetMeshSelectionFunction(const Ref<MeshParticleMeshSelectionMaterialInstance> selfn) {
			customSelectionFunction = selfn;
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
	struct SpritesheetParticleRenderMaterial : public RavEngine::BillboardRenderParticleMaterial {
		SpritesheetParticleRenderMaterial();

	};

	struct SpritesheetParticleRenderMaterialInstance : BillboardParticleRenderMaterialInstance {
		constexpr static uint16_t SpritesheetBindingSlot = 1;
		constexpr static uint16_t SamplerBindingSlot = 0;

		SpritesheetParticleRenderMaterialInstance(Ref<SpritesheetParticleRenderMaterial> mat, uint32_t bytesPerParticle, uint32_t particlePositionOffset, uint32_t particleScaleOffset, uint32_t particleFrameOffset) : BillboardParticleRenderMaterialInstance(mat), bytesPerParticle(bytesPerParticle), particlePositionOffset(particlePositionOffset), particleScaleOffset(particleScaleOffset), particleFrameOffset(particleFrameOffset){
			samplerBindings[SamplerBindingSlot] = true;
		};
		struct spriteCount {
			uint16_t numSpritesWidth = 0;
			uint16_t numSpritesHeight = 0;
		} spriteDim;

		void SetSpritesheet(Ref<Texture> spriteTex) {
			textureBindings[SpritesheetBindingSlot] = spriteTex;
		}

		virtual uint8_t SetPushConstantData(std::span<std::byte, 128> data) const;
	private:
		const uint32_t bytesPerParticle;
		const uint32_t particlePositionOffset;
		const uint32_t particleScaleOffset;
		const uint32_t particleFrameOffset;
	};

	struct PBRMeshParticleRenderMaterial : public MeshParticleRenderMaterial {
		PBRMeshParticleRenderMaterial();
	};

	struct PBRMeshParticleRenderMaterialInstance : public MeshParticleRenderMaterialInstance {
		PBRMeshParticleRenderMaterialInstance(Ref<PBRMeshParticleRenderMaterial> mat, Ref<MeshCollectionStatic> meshes, uint32_t bytesPerParticle, uint32_t positionOffsetBytes) : MeshParticleRenderMaterialInstance(mat, meshes), bytesPerParticle(bytesPerParticle), positionOffsetBytes(positionOffsetBytes){}
		virtual uint8_t SetPushConstantData(std::span<std::byte, 128> data) const;
	private:
		const uint32_t bytesPerParticle;
		const uint32_t positionOffsetBytes;
	};
}