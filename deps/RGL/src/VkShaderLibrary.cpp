#if RGL_VK_AVAILABLE
#include "VkShaderLibrary.hpp"
#include "RGLCommon.hpp"
#include "RGLVk.hpp"
#if __has_include(<librglc.hpp>)
#include <librglc.hpp>
#define RGL_CAN_RUNTIME_COMPILE
#endif
#include <fstream>

static std::vector<uint8_t> readFile(const std::filesystem::path& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("failed to open {}") + filename.string());
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<uint8_t> buffer(fileSize);
	file.seekg(0);
	file.read((char*)(buffer.data()), fileSize);
	file.close();
	return buffer;
}

namespace RGL {
	static void ShaderLibraryFromBytes(VkDevice device, VkShaderModule& module, const std::span<const uint8_t, std::dynamic_extent> code) {
		VkShaderModuleCreateInfo createInfo{
		   .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		   .codeSize = code.size(),    // in bytes, not multiples of uint32
		   .pCode = reinterpret_cast<const uint32_t*>(code.data())
		};
		VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &module))
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
		auto result = librglc::CompileString(source, librglc::API::Vulkan, static_cast<librglc::ShaderStage>(config.stage), {
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