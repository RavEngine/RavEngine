#if !RVE_SERVER
#include "ParticleMaterial.hpp"
#include "App.hpp"

namespace RavEngine {
	ParticleMaterial::ParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName)
	{
		auto device = GetApp()->GetDevice();

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
					}
				},
				.constants = {
					{
						sizeof(ParticleUpdateUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Compute)
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
	}
}

#endif