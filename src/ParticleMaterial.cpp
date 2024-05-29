#if !RVE_SERVER
#include "ParticleMaterial.hpp"
#include "App.hpp"

namespace RavEngine {
	ParticleMaterial::ParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName, const std::string& particleVS, const std::string& particleFS)
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
					.shaderModule = LoadShaderByFilename(Format("{}.csh", initShaderName), device),
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
					.shaderModule = LoadShaderByFilename(Format("{}.csh", updateShaderName), device)
				},
				.pipelineLayout = layout
			});
		}

		// render pipeline
		{
			auto layout = device->CreatePipelineLayout({
				.bindings = {
					{
						.binding = 0,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Vertex,
						.writable = false,
					},
					{
						.binding = 1,
						.type = RGL::BindingType::StorageBuffer,
						.stageFlags = RGL::BindingVisibility::Vertex,
						.writable = false,
					},
					{
						.binding = 2,
						.type = RGL::BindingType::Sampler,
						.stageFlags = RGL::BindingVisibility::Fragment,
						.writable = false,
					},
					{
						.binding = 3,
						.type = RGL::BindingType::SampledImage,
						.stageFlags = RGL::BindingVisibility::Fragment,
						.writable = false,
					}
				},
				.constants = {
					{
						sizeof(ParticleBillboardUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
					}
				}	
			});

			userRenderPipeline = device->CreateRenderPipeline(RGL::RenderPipelineDescriptor{
				.stages = {
					{
						.type = RGL::ShaderStageDesc::Type::Vertex,
						.shaderModule = LoadShaderByFilename(Format("{}.vsh",particleVS),device)
					},
					{
						.type = RGL::ShaderStageDesc::Type::Fragment,
						.shaderModule = LoadShaderByFilename(Format("{}.fsh",particleFS),device)
					},
				},
				.vertexConfig = {
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
				},
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
}

#endif
