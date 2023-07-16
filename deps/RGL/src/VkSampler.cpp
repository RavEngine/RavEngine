#if RGL_VK_AVAILABLE
#include "VkSampler.hpp"
#include "RGLVk.hpp"
#include "VkDevice.hpp"

namespace RGL {

	VkSamplerAddressMode rgl2vksampleraddressmode(SamplerAddressMode mode) {
		switch (mode) {
			case decltype(mode)::Wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case decltype(mode)::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case decltype(mode)::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case decltype(mode)::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case decltype(mode)::MirrorOnce: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}
	}

	SamplerVk::SamplerVk(decltype(owningDevice) owningDevice, const SamplerConfig& config) : owningDevice(owningDevice)
	{
		//TODO: obey config
		VkSamplerCustomBorderColorCreateInfoEXT borderColor{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT,
			.pNext = nullptr,
			.customBorderColor{
				.float32 = {config.borderColor[0],config.borderColor[1],config.borderColor[2],config.borderColor[3]}
			}
		};

		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = &borderColor,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = rgl2vksampleraddressmode(config.addressModeU),
			.addressModeV = rgl2vksampleraddressmode(config.addressModeV),
			.addressModeW = rgl2vksampleraddressmode(config.addressModeW),
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 1,		// can use vkGetPhysicalDeviceProperties --> VkPhysicalDeviceProperties::limits.maxSamplerAnisotropy to determine max value
			.compareEnable = VK_TRUE,
			.compareOp = static_cast<VkCompareOp>(config.compareFunction),
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT,
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