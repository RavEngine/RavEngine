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

	VkSamplerMipmapMode rgl2vkmipmode(MipFilterMode mode) {
		switch (mode) {
		case decltype(mode)::NotMipped: 
		case decltype(mode)::Nearest:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case decltype(mode)::Linear:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			FatalError("Unsupported mip sampler");
		}
	}

	VkFilter rgl2vkfilter(MinMagFilterMode mode) {
		switch (mode) {
		case decltype(mode)::Nearest:
			return VK_FILTER_NEAREST;
		case decltype(mode)::Linear:
			return VK_FILTER_LINEAR;
		}
	}

	VkSamplerReductionMode rgl2vkreduction(SamplerReductionMode mode) {
		switch (mode) {
		case decltype(mode)::Standard: 
		case decltype(mode)::Comparison:
			return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
		case decltype(mode)::Minimum:
			return VK_SAMPLER_REDUCTION_MODE_MIN;
		case decltype(mode)::Maximum:
			return VK_SAMPLER_REDUCTION_MODE_MAX;
		}
	}

	SamplerVk::SamplerVk(decltype(owningDevice) owningDevice, const SamplerConfig& config_orig) : owningDevice(owningDevice)
	{
		auto config = config_orig;

		VkSamplerReductionModeCreateInfoEXT createInfoReduction = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT,
			.pNext = nullptr,
			.reductionMode = rgl2vkreduction(config.reductionMode)
		};

		VkSamplerCustomBorderColorCreateInfoEXT borderColor{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT,
			.pNext = &createInfoReduction,
			.customBorderColor{
				.float32 = {config.borderColor[0],config.borderColor[1],config.borderColor[2],config.borderColor[3]}
			}
		};

		if (config.compareFunction == RGL::DepthCompareFunction::None) {
			config.compareFunction = RGL::DepthCompareFunction::Always;
		}

		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = &borderColor,
			.magFilter = rgl2vkfilter(config.magFilter),
			.minFilter = rgl2vkfilter(config.minFilter),
			.mipmapMode = rgl2vkmipmode(config.mipFilter),
			.addressModeU = rgl2vksampleraddressmode(config.addressModeU),
			.addressModeV = rgl2vksampleraddressmode(config.addressModeV),
			.addressModeW = rgl2vksampleraddressmode(config.addressModeW),
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 1,		// can use vkGetPhysicalDeviceProperties --> VkPhysicalDeviceProperties::limits.maxSamplerAnisotropy to determine max value
			.compareEnable = config.compareFunction != DepthCompareFunction::Always,
			.compareOp = static_cast<VkCompareOp>(config.compareFunction),
			.minLod = 0.0f,
			.maxLod = VK_LOD_CLAMP_NONE,
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