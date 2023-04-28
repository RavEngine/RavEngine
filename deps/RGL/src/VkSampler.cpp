#if RGL_VK_AVAILABLE
#include "VkSampler.hpp"
#include "RGLVk.hpp"
#include "VkDevice.hpp"

namespace RGL {
	SamplerVk::SamplerVk(decltype(owningDevice) owningDevice, const SamplerConfig& config) : owningDevice(owningDevice)
	{
		//TODO: obey config
		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 1,		// can use vkGetPhysicalDeviceProperties --> VkPhysicalDeviceProperties::limits.maxSamplerAnisotropy to determine max value
			.compareEnable = VK_TRUE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VK_CHECK(vkCreateSampler(owningDevice->device, &samplerInfo, nullptr, &sampler));
	}
	SamplerVk::~SamplerVk()
	{
		vkDestroySampler(owningDevice->device, sampler, nullptr);
	}
}
#endif