#if !RVE_SERVER
#include "ParticleMaterial.hpp"
#include "App.hpp"
#include "Texture.hpp"
#include <RGL/Texture.hpp>

namespace RavEngine {
	const RGL::RenderPipelineDescriptor::VertexConfig quadVertexConfig{
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


	ParticleRenderMaterial::ParticleRenderMaterial(const std::string_view particleVS, const std::string_view particleFS, const InternalConfig& internalConfig, const ParticleRenderMaterialConfig& config) : opacityMode(config.opacityMode)
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
                    {
                        .binding = particleEmitterStateBufferBinding,
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

			RGL::RenderPipelineDescriptor rpd{
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
					.topology = internalConfig.topology,
				},
				.rasterizerConfig = {
					.windingOrder = RGL::WindingOrder::Counterclockwise,
				},
				.colorBlendConfig = internalConfig.mode == LightingMode::Lit ? (
					config.opacityMode == OpacityMode::Opaque ? RavEngine::defaultColorBlendConfig : RavEngine::defaultTransparentColorBlendConfig
					) : (
						config.opacityMode == OpacityMode::Opaque ? RavEngine::defaultUnlitColorBlendConfig : RavEngine::defaultTransparentUnlitColorBlendConfig
						),
				.depthStencilConfig = {
					.depthFormat = RGL::TextureFormat::D32SFloat,
					.depthTestEnabled = config.zTestEnabled,
					.depthWriteEnabled = IsTransparent() ? false : config.zWriteEnabled,
					.depthFunction = RGL::DepthCompareFunction::Greater,
				},
				.pipelineLayout = layout
			};

			userRenderPipeline = device->CreateRenderPipeline(rpd);
			rpd.colorBlendConfig.attachments.clear();					// no color attachments for shadow mode
			shadowRenderPipeline = device->CreateRenderPipeline(rpd);
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

		auto dim = textureBindings[SpritesheetParticleRenderMaterial<LightingMode::Lit>::SpritesheetBindingSlot]->GetRHITexturePointer()->GetSize();
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
					.binding = 7,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
			},
			.pushConstantSize = sizeof(PBRUBO)
		}
	) {}


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
		textureBindings[kEmissiveBinding] = Texture::Manager::zeroTexture;
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

extern std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> augmentLitMaterialBindings(const std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc>& bindings, OpacityMode opacityMode);

extern std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> augmentUnlitMaterialBindings(const std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc>& bindings, OpacityMode opacityMode);

	ParticleRenderMaterialConfig MeshParticleRenderMaterial_Impl::augmentMeshConfig(const ParticleRenderMaterialConfig& in, LightingMode mode)
	{
		auto cpy = in;

        if (mode == LightingMode::Lit){
            cpy.bindings = augmentLitMaterialBindings(cpy.bindings, in.opacityMode);
        }
        else{
            cpy.bindings = augmentUnlitMaterialBindings(cpy.bindings, in.opacityMode);
        }
		cpy.bindings.push_back({
					.binding = 21,									// engine-required binding
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Vertex,
					.writable = false
			});

		return cpy;
	}
	ParticleRenderMaterialConfig BillboardRenderParticleMaterial_impl::augmentBillboardConfig(const ParticleRenderMaterialConfig& in, LightingMode mode)
	{
		auto cpy = in;

        if (mode == LightingMode::Lit){
            cpy.bindings = augmentLitMaterialBindings(cpy.bindings, in.opacityMode);
        }
        else{
            cpy.bindings = augmentUnlitMaterialBindings(cpy.bindings, in.opacityMode);
        }

		return cpy;
	}
}

#endif
