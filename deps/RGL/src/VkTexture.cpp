#if RGL_VK_AVAILABLE
#include "VkTexture.hpp"
#include "VkDevice.hpp"
#include "RGLVk.hpp"
#include <cstring>

namespace RGL {

	VkImageAspectFlags rgl2vkAspectFlags(TextureAspect rglAspect) {
		VkImageAspectFlags aspect = 0;

		if (rglAspect.HasColor) {
			aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (rglAspect.HasDepth) {
			aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (rglAspect.HasStencil) {
			aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		if (rglAspect.HasMetadata) {
			aspect |= VK_IMAGE_ASPECT_METADATA_BIT;
		}

		return aspect;
	}

	VkImageUsageFlags rgl2vkUsageFlags(TextureUsage rglUsage) {
		VkImageUsageFlags usage = 0;

		if (rglUsage.ColorAttachment) {
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (rglUsage.DepthStencilAttachment) {
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		if (rglUsage.InputAttachment) {
			usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}
		if (rglUsage.Sampled) {
			usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (rglUsage.Storage) {
			usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		if (rglUsage.TransferDestination) {
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (rglUsage.TransferSource) {
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (rglUsage.TransientAttachment) {
			usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		}

		return usage;
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
	}

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer,graphicsQueue,device,commandPool);
	}

	TextureVk::TextureVk(decltype(vkImageView) imageView, decltype(vkImage) image, const Dimension& size) : vkImageView(imageView), vkImage(image), ITexture(size)
	{
	}
	TextureVk::TextureVk(decltype(owningDevice) owningDevice, const TextureConfig& config, untyped_span bytes) : TextureVk(owningDevice, config)
	{
		// allocate a staging buffer for the texture
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		auto allocation = createBuffer(owningDevice.get(), bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);

		auto device = owningDevice->device;


		// put the data in the buffer
		void* data;
		vmaMapMemory(owningDevice->vkallocator, allocation, &data);
		memcpy(data, bytes.data(), bytes.size());
		vmaUnmapMemory(owningDevice->vkallocator, allocation);

		auto format = RGL2VkTextureFormat(config.format);

		// TODO: these probably should share the same command buffer
		transitionImageLayout(vkImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, owningDevice->commandPool, owningDevice->presentQueue);

		copyBufferToImage(stagingBuffer, vkImage, static_cast<uint32_t>(config.width), static_cast<uint32_t>(config.height), device, owningDevice->commandPool, owningDevice->presentQueue);

		transitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,device, owningDevice->commandPool, owningDevice->presentQueue);

		// cleanup
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vmaFreeMemory(owningDevice->vkallocator, allocation);
	}
	TextureVk::TextureVk(decltype(owningDevice) owningDevice, const TextureConfig& config) : owningDevice(owningDevice), owning(true), ITexture(Dimension{ .width = config.width,.height = config.height })
	{
		const auto format = RGL2VkTextureFormat(config.format);

		//TODO: read other options from config
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = {
				.width = config.width,
				.height = config.height,
				.depth = config.depth,
			},
			.mipLevels = config.mipLevels,
			.arrayLayers = config.arrayLayers,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = rgl2vkUsageFlags(config.usage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = rgl2vkImageLayout(config.initialLayout),
		};

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		VmaAllocationInfo allocInfo;
		VK_CHECK(vmaCreateImage(owningDevice->vkallocator, &imageInfo, &allocCreateInfo, &vkImage, &alloc, &allocInfo));	// also binds memory

		VkImageViewCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = vkImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components{
			.r = VK_COMPONENT_SWIZZLE_IDENTITY, // we don't want any swizzling
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
	},
		.subresourceRange{
			.aspectMask = rgl2vkAspectFlags(config.aspect),    // mipmap and layer info (we don't want any here)
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
		};
		VK_CHECK(vkCreateImageView(owningDevice->device, &createInfo, nullptr, &vkImageView));

		if (config.debugName) {
			owningDevice->SetDebugNameForResource(vkImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, config.debugName);
		}

	}
	Dimension TextureVk::GetSize() const
	{
		return this->size;
	}
	TextureVk::~TextureVk()
	{
		if (owning) {
			vkDestroyImage(owningDevice->device, vkImage, nullptr);
			vkDestroyImageView(owningDevice->device, vkImageView, nullptr);
			vmaFreeMemory(owningDevice->vkallocator, alloc);
			alloc = VK_NULL_HANDLE;
		}
	}
}

#endif