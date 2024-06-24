#if !RVE_SERVER
#include "ParticleMaterial.hpp"
#include "App.hpp"
#include "Texture.hpp"
#include <RGL/Texture.hpp>

namespace RavEngine {
	static const RGL::RenderPipelineDescriptor::VertexConfig quadVertexConfig{
		.vertexBindings = {
			{
				.binding = 0,
				.stride = sizeof(ParticleQuadVert)
			}
		},
		.attributeDescs = {
			{
				.location = 0,
				.binding = 0,
				.offset = offsetof(ParticleQuadVert,pos),
				.format = RGL::VertexAttributeFormat::R32G32_SignedFloat
			}
		}
	};


	ParticleRenderMaterial::ParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const InternalConfig& internalConfig, const ParticleRenderMaterialConfig& config)
	{
		auto device = GetApp()->GetDevice();
		// render pipeline
		{
			RGL::PipelineLayoutDescriptor rpl = {
				.bindings = {
					{
						.binding = particleDataBufferBinding,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Vertex,
						.writable = false,
					},
					{
						.binding = particleAliveIndexBufferBinding,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Vertex,
						.writable = false,
					},
					{
						.binding = particleMatrixBufferBinding,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Vertex,
						.writable = false,
					},

				},
			};

			if (config.pushConstantSize > 0) {
				rpl.constants = {
					{
						config.pushConstantSize, 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
					}
				};
			}

			for (const auto& binding : config.bindings) {
				rpl.bindings.push_back(binding);
			}

			auto layout = device->CreatePipelineLayout(rpl);

			userRenderPipeline = device->CreateRenderPipeline(RGL::RenderPipelineDescriptor{
				.stages = {
					{
						.type = RGL::ShaderStageDesc::Type::Vertex,
						.shaderModule = LoadShaderByFilename(Format("{}_vsh",particleVS),device)
					},
					{
						.type = RGL::ShaderStageDesc::Type::Fragment,
						.shaderModule = LoadShaderByFilename(Format("{}_fsh",particleFS),device)
					},
				},
				.vertexConfig = internalConfig.vertexConfig,
				.inputAssembly = {
					.topology = RGL::PrimitiveTopology::TriangleStrip,
				},
				.rasterizerConfig = {
					.windingOrder = RGL::WindingOrder::Counterclockwise,
				},
				.colorBlendConfig = RavEngine::defaultColorBlendConfig,
				.depthStencilConfig = {
					.depthFormat = RGL::TextureFormat::D32SFloat,
					.depthTestEnabled = true,
					.depthWriteEnabled = true,
					.depthFunction = RGL::DepthCompareFunction::Greater,
				},
				.pipelineLayout = layout
			});
		}
	}
	ParticleUpdateMaterial::ParticleUpdateMaterial(const std::string_view initShaderName, const std::string_view updateShaderName)
	{
		auto device = GetApp()->GetDevice();
		// init shader
		{
			auto layout = device->CreatePipelineLayout({
				.bindings = {
					{
						.binding = 0,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = false,
					},
					{
						.binding = 1,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = false,
					},
					{
						.binding = 2,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = true,
					},
					{
						.binding = 3,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = true,
					},
					{
						.binding = 4,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = false,
					}
				},
				.constants = {}
				});

			userInitPipeline = device->CreateComputePipeline({
				.stage = {
					.type = RGL::ShaderStageDesc::Type::Compute,
					.shaderModule = LoadShaderByFilename(Format("{}", initShaderName), device),
				},
				.pipelineLayout = layout
				});
		}
		// update shader
		{
			auto layout = device->CreatePipelineLayout({
				.bindings = {
					{
						.binding = 0,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = true,
					},
					{
						.binding = 1,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = true,
					},
					{
						.binding = 2,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = true,
					},
					{
						.binding = 3,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Compute,
						.writable = true,
					},
				},
				.constants = {
					{
						sizeof(ParticleUpdateUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Compute)				//TODO: don't hardcode the constants size, this is dependent on billboard vs mesh render mode
					}
				}
				});

			userUpdatePipeline = device->CreateComputePipeline({
				.stage = {
					.type = RGL::ShaderStageDesc::Type::Compute,
					.shaderModule = LoadShaderByFilename(Format("{}", updateShaderName), device)
				},
				.pipelineLayout = layout
				});
		}
	}
	uint8_t SpritesheetParticleRenderMaterialInstance::SetPushConstantData(std::span<std::byte, 128> data) const
	{
		ParticleBillboardUBO str{
			.spritesheetDim = {},
			.numSprites = {},
			.bytesPerParticle = bytesPerParticle,
			.particlePositionOffset = particlePositionOffset,
			.particleScaleOffset = particleScaleOffset,
			.particleFrameOffset = particleFrameOffset
		};

		auto dim = textureBindings[SpritesheetBindingSlot]->GetRHITexturePointer()->GetSize();
		str.spritesheetDim = {
				dim.width,
				dim.height,
		};
		str.numSprites = {
				spriteDim.numSpritesWidth,
				spriteDim.numSpritesHeight,
		};

		std::memcpy(data.data(), &str, sizeof(str));
		return sizeof(str);
	}

	SpritesheetParticleRenderMaterial::SpritesheetParticleRenderMaterial() : BillboardRenderParticleMaterial("default_billboard_particle", "default_billboard_particle", {
			.bindings = {
				{
					.binding = SpritesheetParticleRenderMaterialInstance::SamplerBindingSlot,
					.type = RGL::BindingType::Sampler,
					.stageFlags = RGL::BindingVisibility::Fragment,
					.writable = false,
				},
				{
					.binding = SpritesheetParticleRenderMaterialInstance::SpritesheetBindingSlot,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
					.writable = false,
				}
			},
			.pushConstantSize = sizeof(ParticleBillboardUBO)
		}) {}

	struct PBRUBO {
		glm::vec4 colorTint = {1.0f,1,1,1};
		float metallicTint = 0.6;
		float roughnessTint = 0.5;
		float specularTint = 0;
		uint32_t bytesPerParticle;
		uint32_t positionOffset;
		uint32_t scaleOffset;
		uint32_t rotationOffset;
	};

	PBRMeshParticleRenderMaterial::PBRMeshParticleRenderMaterial() : MeshParticleRenderMaterial("pbr_particle", "pbr_particle",
		{
			.bindings = {
				{
					.binding = 0,
					.type = RGL::BindingType::Sampler,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 1,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 2,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 3,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 4,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 5,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 6,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},

				{
					.binding = 11,									// engine-required binding
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Vertex,
					.writable = false
				}
			},
			.pushConstantSize = sizeof(PBRUBO)
		}
	) {}

	BillboardRenderParticleMaterial::BillboardRenderParticleMaterial(const std::string_view particleVS, const std::string_view particleFS, const ParticleRenderMaterialConfig& config) : ParticleRenderMaterial(particleFS, particleVS, {
			.vertexConfig = quadVertexConfig
		}, config) {}
	MeshParticleRenderMaterial::MeshParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const ParticleRenderMaterialConfig& config) : ParticleRenderMaterial(particleVS, particleFS, {
		.vertexConfig = defaultVertexConfig
	}, config) {}


	MeshParticleMeshSelectionMaterial::MeshParticleMeshSelectionMaterial(const std::string_view name)
	{
		auto device = GetApp()->GetDevice();

		auto layout = device->CreatePipelineLayout({
			.bindings = {
				{
					.binding = 10,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = true
				},
				{
					.binding = 11,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = true
				},
				{
					.binding = 12,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 13,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 14,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 15,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				}
			}
		});

		userSelectionPipeline = device->CreateComputePipeline({
			.stage = {
				.type = RGL::ShaderStageDesc::Type::Compute,
				.shaderModule = LoadShaderByFilename(Format("{}", name), device),
			},
			.pipelineLayout = layout
		});
	}

	PBRMeshParticleRenderMaterialInstance::PBRMeshParticleRenderMaterialInstance(Ref<PBRMeshParticleRenderMaterial> mat, Ref<MeshCollectionStatic> meshes, uint32_t bytesPerParticle, uint32_t positionOffsetBytes, uint32_t scaleOffsetBytes, uint32_t rotationOffsetBytes) : MeshParticleRenderMaterialInstance(mat, meshes), bytesPerParticle(bytesPerParticle), positionOffsetBytes(positionOffsetBytes), scaleOffsetBytes(scaleOffsetBytes), rotationOffsetBytes(rotationOffsetBytes)
	{
		samplerBindings[kSamplerBinding] = true;
		textureBindings[kDiffuseBinding] = Texture::Manager::defaultTexture;
		textureBindings[kNormalBinding] = Texture::Manager::defaultNormalTexture;
		textureBindings[kSpecularBinding] = Texture::Manager::defaultTexture;
		textureBindings[kMetallicBinding] = Texture::Manager::defaultTexture;
		textureBindings[kRoughnessBinding] = Texture::Manager::defaultTexture;
		textureBindings[kAOBinding] = Texture::Manager::defaultTexture;
	}

	uint8_t PBRMeshParticleRenderMaterialInstance::SetPushConstantData(std::span<std::byte, 128> data) const
	{
		PBRUBO ubo{
			.bytesPerParticle = bytesPerParticle,
			.positionOffset = positionOffsetBytes,
			.scaleOffset = scaleOffsetBytes,
			.rotationOffset = rotationOffsetBytes
		};

		std::memcpy(data.data(), &ubo, sizeof(ubo));
		return sizeof(ubo);
	}
}

#endif
