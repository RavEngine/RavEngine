#pragma once
#include <RGL/Types.hpp>
#include <RGL/ShaderLibrary.hpp>
#include "VkDevice.hpp"
#include <volk.h>
#include <span>
#include <filesystem>
#include "VkPipelineLibraryShared.hpp"

namespace RGL {
	struct ShaderLibraryVk : public IShaderLibrary {
		const std::shared_ptr<DeviceVk> owningDevice;
		VkShaderModule shaderModule = VK_NULL_HANDLE;

		BufferBindingStore bindingInfo;

		ShaderLibraryVk(decltype(owningDevice));
		ShaderLibraryVk(decltype(owningDevice), const std::span<const uint8_t, std::dynamic_extent>);
		ShaderLibraryVk(decltype(owningDevice), const std::string_view, const FromSourceConfig& config);
		ShaderLibraryVk(decltype(owningDevice), const std::filesystem::path& path);

		virtual ~ShaderLibraryVk();
	private:
		void ShaderLibraryFromBytes(VkDevice device, VkShaderModule& module, const std::span<const uint8_t, std::dynamic_extent> code);
	};
}