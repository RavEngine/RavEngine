# pragma once

#include <RGL/Types.hpp>
#include "RGLCommon.hpp"
#include <RGL/TextureFormat.hpp>
#include <RGL/Pipeline.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h> 
#include <cassert>

#define VK_CHECK(a) {auto VK_CHECK_RESULT = a; Assert(VK_CHECK_RESULT == VK_SUCCESS, std::string("Vulkan assertion failed: ") + # a + " -> " + string_VkResult(VK_CHECK_RESULT));}
#define VK_VALID(a) {assert(a != VK_NULL_HANDLE);}

constexpr bool enableValidationLayers =
#ifdef NDEBUG
false;
#else
true;
#endif

constexpr static const char* const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};


namespace RGL {
	void InitVk(const InitOptions&);
	void DeinitVk();

	struct DeviceVk;

	extern VkInstance instance;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface);

	enum class MSASampleCount : uint8_t;
	VkSampleCountFlagBits RGLMSA2VK(const RGL::MSASampleCount& samplecount);

	VkFormat RGL2VkTextureFormat(TextureFormat);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

	VmaAllocation createBuffer(DeviceVk*, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer);

	VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkDevice device, VkCommandPool commandPool);

	RGLRenderPassPtr CreateRenderPassVk(const RenderPassConfig& config);

	VkImageLayout rgl2vkImageLayout(RGL::ResourceLayout layout);

	VkShaderStageFlagBits RGL2VKshader(RGL::ShaderStageDesc::Type type);
}