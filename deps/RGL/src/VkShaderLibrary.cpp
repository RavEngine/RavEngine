#if RGL_VK_AVAILABLE
#include "VkShaderLibrary.hpp"
#include "RGLCommon.hpp"
#include "RGLVk.hpp"
#if __has_include(<librglc.hpp>)
#include <librglc.hpp>
#define RGL_CAN_RUNTIME_COMPILE 1
#endif
#include <fstream>
#include <spirv_reflect.h>

static std::vector<uint8_t> readFile(const std::filesystem::path& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		RGL::FatalError(std::string("failed to open {}") + filename.string());
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<uint8_t> buffer(fileSize);
	file.seekg(0);
	file.read((char*)(buffer.data()), fileSize);
	file.close();
	return buffer;
}

namespace RGL {
	 void ShaderLibraryVk::ShaderLibraryFromBytes(VkDevice device, VkShaderModule& module, const std::span<const uint8_t, std::dynamic_extent> code) {
		VkShaderModuleCreateInfo createInfo{
		   .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		   .codeSize = code.size(),    // in bytes, not multiples of uint32
		   .pCode = reinterpret_cast<const uint32_t*>(code.data())
		};
		VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &module));

		// get reflection data
		SpvReflectShaderModule spvModule;
		SpvReflectResult result = spvReflectCreateShaderModule(code.size_bytes(), code.data(), &spvModule);
		if (result != SPV_REFLECT_RESULT_SUCCESS) {
			FatalError("SPIRV reflection capture failed");
		}

		
		auto setdata = spvReflectGetDescriptorSet(&spvModule, 0, &result);
		bindingInfo = std::make_shared<decltype(bindingInfo)::element_type>();
		if (result == SPV_REFLECT_RESULT_SUCCESS) {	// otherwise there are no descriptor sets
			for (uint32_t i = 0; i < setdata->binding_count; i++) {
				switch (setdata->bindings[i]->descriptor_type) {
				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				{
					bool writable = (setdata->bindings[i]->resource_type & SPV_REFLECT_RESOURCE_FLAG_UAV);
					auto slot = setdata->bindings[i]->binding;
					(*bindingInfo)[slot] = { writable };
				}
				break;
				default:
					break;
				}
			}
		}
	}

	ShaderLibraryVk::ShaderLibraryVk(decltype(owningDevice) device) : owningDevice(device)
	{
		FatalError("Default library is not available on Vulkan");
	}
	ShaderLibraryVk::ShaderLibraryVk(decltype(owningDevice) device, const std::span<const uint8_t, std::dynamic_extent> code): owningDevice(device)
	{
		ShaderLibraryFromBytes(owningDevice->device,shaderModule, code);
	}
	ShaderLibraryVk::ShaderLibraryVk(decltype(owningDevice) device, const std::string_view source, const FromSourceConfig& config) : owningDevice(device)
	{
#if RGL_CAN_RUNTIME_COMPILE
		auto result = librglc::CompileString(source, "rumtime_shader", librglc::API::Vulkan, static_cast<librglc::ShaderStage>(config.stage), {
			.outputBinary = true,
			.entrypointOutputName = "main"
		});
		
		ShaderLibraryFromBytes(owningDevice->device, shaderModule, std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(result.data()), result.size()));
#else
		FatalError("RGL was not built with runtime shader compilation support");
#endif
	}
	ShaderLibraryVk::ShaderLibraryVk(decltype(owningDevice) device, const std::filesystem::path& path) : owningDevice(device)
	{
		auto bindata = readFile(path);
		ShaderLibraryFromBytes(owningDevice->device, shaderModule, bindata);
	}
	ShaderLibraryVk::~ShaderLibraryVk()
	{
		vkDestroyShaderModule(owningDevice->device, shaderModule, nullptr);
	}
}

#endif