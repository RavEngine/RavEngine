#if !RVE_SERVER
#include "ParticleMaterial.hpp"
#include "App.hpp"

namespace RavEngine {
	ParticleMaterial::ParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName)
	{
		auto device = GetApp()->GetDevice();

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

		userInitShader = device->CreateComputePipeline({
			.stage = {
				.shaderModule = LoadShaderByFilename(Format("{}.csh", initShaderName), device)
			},
			.pipelineLayout = layout
		});
	}
}

#endif