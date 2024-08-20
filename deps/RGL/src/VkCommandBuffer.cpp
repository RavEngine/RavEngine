#if RGL_VK_AVAILABLE
#include "VkCommandBuffer.hpp"
#include "VkCommandQueue.hpp"
#include "VkDevice.hpp"
#include "VkRenderPipeline.hpp"
#include "VkTexture.hpp"
#include "VkBuffer.hpp"
#include "VkRenderPass.hpp"
#include "VkSampler.hpp"
#include "VkSwapchain.hpp"
#include "VkComputePipeline.hpp"
#include <cstring>
#include <iostream>
#include <utility>
#include <vk_mem_alloc.h>

namespace RGL {
	VkAttachmentLoadOp RGL2LoadOp(LoadAccessOperation op) {
		switch (op) {
		case decltype(op)::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case decltype(op)::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case decltype(op)::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case decltype(op)::NotAccessed: return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
		}
	}

	VkAttachmentStoreOp RGL2StoreOp(StoreAccessOperation op) {
		switch (op) {
		case decltype(op)::Store: return VK_ATTACHMENT_STORE_OP_STORE;
		case decltype(op)::None: return VK_ATTACHMENT_STORE_OP_NONE;
		case decltype(op)::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
	}

	void encodeResourceTransition(VkCommandBuffer commandBuffer, VkImage image,
		decltype(VkImageMemoryBarrier::srcAccessMask) srcAccessMask,
		decltype(VkImageMemoryBarrier::dstAccessMask) dstAccessMask,
		decltype(VkImageMemoryBarrier::oldLayout) oldLayout,
		decltype(VkImageMemoryBarrier::newLayout) newLayout,
		decltype(decltype(VkImageMemoryBarrier::subresourceRange)::aspectMask) aspectMask,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask
	) {

	}

	void RGL::CommandBufferVk::Reset()
	{
		VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));
		vkResetFences(owningQueue->owningDevice->device, 1, &internalFence);
	}
	void CommandBufferVk::Begin()
	{
		VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr,
		};
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo))
	}
	void CommandBufferVk::End()
	{
		// ensure that all commands have been encoded
		if (renderCommands.size() > 0) {
			EncodeQueuedCommands();
		}

		for (const auto swapRsc : swapchainImages) {
			RecordTextureBinding(TextureView{ swapRsc,swapRsc->GetDefaultView().texture.vk.view, ALL_MIPS, ALL_LAYERS, swapRsc->GetSize() }, { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR , true });
		}

		// place resources back in their native state
		for (const auto [texture, written] : activeTextures) {
			if (!swapchainImages.contains(texture.texture)) {
				TextureView view{ texture.texture,texture.texture->GetDefaultView().texture.vk.view, ALL_MIPS, ALL_LAYERS, texture.texture->GetSize() };
				RecordTextureBinding(view, { texture.texture->nativeFormat , true });
			}
		}
		swapchainImages.clear();
		activeTextures.clear();

		VK_CHECK(vkEndCommandBuffer(commandBuffer));
		activeBuffers.clear();
	}
	void CommandBufferVk::BindRenderPipeline(RGLRenderPipelinePtr generic_pipeline)
	{
		currentRenderPipeline = std::static_pointer_cast<RenderPipelineVk>(generic_pipeline);
		EncodeCommand(CmdBindRenderPipeline{ generic_pipeline });
	}
	void CommandBufferVk::BeginRendering(RGLRenderPassPtr renderPassPtr)
	{
		isInsideRenderingBlock = true;
		EncodeCommand(CmdBeginRendering{ renderPassPtr });

		auto renderPass = std::static_pointer_cast<RenderPassVk>(renderPassPtr);
		uint32_t i = 0;
		for (const auto& attachment : renderPass->config.attachments) {
			RecordTextureBinding(renderPass->textures[i], {
					.lastLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.written = true
				}
			);
			i++;
		}

		if (renderPass->config.depthAttachment.has_value()) {
			RecordTextureBinding(renderPass->depthTexture.value(),
				{
					.lastLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
					.written = true
				}
			);
		}

		if (renderPass->config.stencilAttachment.has_value()) {
			RecordTextureBinding(renderPass->stencilTexture.value(),
				{
					.lastLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
					.written = true
				}
			);
		}
	}

	void CommandBufferVk::EndRendering()
	{
		EncodeQueuedCommands();
		vkCmdEndRendering(commandBuffer);
		currentRenderPipeline = nullptr;	// reset this to avoid having stale state
		isInsideRenderingBlock = false;
	}
	void CommandBufferVk::BeginCompute(RGLComputePipelinePtr inPipeline)
	{
		currentComputePipeline = std::static_pointer_cast<ComputePipelineVk>(inPipeline);
		EncodeCommand(CmdBeginCompute{ inPipeline });
	}
	void CommandBufferVk::EndCompute()
	{
		EncodeCommand(CmdEndCompute{});
		EncodeQueuedCommands();
		currentComputePipeline = nullptr;
	}
	void CommandBufferVk::DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX, uint32_t threadsPerThreadgroupY, uint32_t threadsPerThreadgroupZ)
	{
		EncodeCommand(CmdDispatch{ threadsX, threadsY, threadsZ });
	}
	void CommandBufferVk::BindBuffer(RGLBufferPtr buffer, uint32_t bindingOffset, uint32_t offsetIntoBuffer)
	{
		GenericBindBuffer(buffer, offsetIntoBuffer, bindingOffset, VK_PIPELINE_BIND_POINT_GRAPHICS);
	}

	void CommandBufferVk::GenericBindBuffer(RGLBufferPtr& buffer, const uint32_t& offsetIntoBuffer, const uint32_t& bindingOffset, VkPipelineBindPoint bindPoint)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(buffer).get(), { .written = IsBufferSlotWritable(bindingOffset) });
		EncodeCommand(CmdBindBuffer{ buffer, offsetIntoBuffer, bindingOffset, bindPoint });
	}

	void CommandBufferVk::BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer)
	{
		GenericBindBuffer(buffer, offsetIntoBuffer, binding, VK_PIPELINE_BIND_POINT_COMPUTE);
	}

	void CommandBufferVk::SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(buffer).get(), { .written = false });
		EncodeCommand(CmdSetVertexBuffer{ buffer,bindingInfo });
	}

	void CommandBufferVk::setPushConstantData(const RGL::untyped_span& data, const uint32_t& offset)
	{
		Assert(data.size() <= 128, "Push constant data size must be no more than 128 bytes");
		CmdSetPushConstantData cmd{
			// size must be a multiple of 4
			// need to get a little extra space for safety
			.size = uint32_t((data.size() + (data.size() % 4 != 0 ? 4 : 0))),
			.offset = offset,
		};
		std::memcpy(cmd.data, data.data(), data.size());
		EncodeCommand(cmd);
	}

	void CommandBufferVk::SetVertexBytes(const untyped_span data, uint32_t offset)
	{
		setPushConstantData(data, offset);
	}
	void CommandBufferVk::SetFragmentBytes(const untyped_span data, uint32_t offset)
	{
		setPushConstantData(data, offset);
	}
	void CommandBufferVk::SetComputeBytes(const untyped_span data, uint32_t offset)
	{
		setPushConstantData(data, offset);
	}
	void CommandBufferVk::SetIndexBuffer(RGLBufferPtr buffer)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(buffer).get(), { .written = false });
		EncodeCommand(CmdSetIndexBuffer{ buffer });
	}
	void CommandBufferVk::SetVertexSampler(RGLSamplerPtr sampler, uint32_t index)
	{
		EncodeCommand(CmdSetSampler{ sampler, index });
	}
	void CommandBufferVk::SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index)
	{
		EncodeCommand(CmdSetSampler{ sampler, index });
	}
	void CommandBufferVk::SetComputeSampler(RGLSamplerPtr sampler, uint32_t index)
	{
		EncodeCommand(CmdSetSampler{ sampler, index, true });
	}
	void CommandBufferVk::SetVertexTexture(const TextureView& texture, uint32_t index)
	{
		SetFragmentTexture(texture, index);
	}
	void CommandBufferVk::SetFragmentTexture(const TextureView& texture, uint32_t index)
	{

		auto vktexture = static_cast<const TextureVk*>(texture.parent);

		if (vktexture == nullptr) {

			EncodeCommand(CmdBindlessSetTexture{
				.set = texture.texture.vk.bindlessInfo.bindlessSet,
				.binding = index
			});

			return;
		}

		auto nextLayout = vktexture->createdConfig.usage.DepthStencilAttachment ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		auto activeLayout = currentRenderPipeline ? currentRenderPipeline->pipelineLayout : currentComputePipeline->pipelineLayout;
		if (activeLayout->bindingDescriptorTypes.at(index) == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
			nextLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		RecordTextureBinding(texture, {
			.lastLayout = nextLayout,
			.written = true
			});

		EncodeCommand(CmdSetTexture{ texture, index });
	}
	void CommandBufferVk::SetComputeTexture(const TextureView& texture, uint32_t index)
	{
		SetFragmentTexture(texture, index);
	}

	void CommandBufferVk::Draw(uint32_t nVertices, const DrawInstancedConfig& config)
	{
		EncodeCommand(CmdDraw{ nVertices, config });
	}
	void CommandBufferVk::DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig& config)
	{
		EncodeCommand(CmdDrawIndexed{ nIndices, config });
	}

	void CommandBufferVk::CopyTextureToBuffer(TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr destBuffer)
	{
		auto castedDest = std::static_pointer_cast<BufferVk>(destBuffer);

		EncodeCommand(CmdCopyTextureToBuffer{
			sourceTexture,
			sourceRect,
			offset,
			destBuffer
			});

		RecordTextureBinding(sourceTexture, { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, true });
	}
	void CommandBufferVk::CopyBufferToTexture(RGLBufferPtr source, uint32_t size, const TextureDestConfig& dest)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(source).get(), { .written = false });
		RecordTextureBinding(dest.view, { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL });

		EncodeCommand(CmdCopyBuffertoTexture{
			.srcBuffer = source,
			.nBytes = size,
			.destTexture = dest.view,
			.destLoc = dest.destLoc,
			.arrayLayer = dest.arrayLayer
		});
	}
	void CommandBufferVk::CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(from.buffer).get(), { .written = false });
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(to.buffer).get(), { .written = true });
		EncodeCommand(CmdCopyBufferToBuffer{ from,to,size });
	}
	void CommandBufferVk::CopyTextureToTexture(const TextureCopyConfig& from, const TextureCopyConfig& to)
	{
		auto fromCpy = from.texture;
		fromCpy.texture.vk.coveredLayers = MakeMipMaskForIndex(from.layer);
		fromCpy.texture.vk.coveredMips = MakeMipMaskForIndex(from.mip);

		auto toCpy = to.texture;
		toCpy.texture.vk.coveredLayers = MakeMipMaskForIndex(to.layer);
		toCpy.texture.vk.coveredMips = MakeMipMaskForIndex(to.mip);

		RecordTextureBinding(fromCpy, {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL}, false);
		RecordTextureBinding(toCpy, { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL }, false);

		auto fromcnfcpy = from;
		fromcnfcpy.texture = fromCpy;

		auto tocnfcpy = to;
		tocnfcpy.texture = toCpy;

		EncodeCommand(CmdCopyTextureToTexture{ fromcnfcpy, tocnfcpy, from.mip, from.layer, to.mip, to.layer });
	}
	void CommandBufferVk::SetViewport(const Viewport& viewport)
	{
		EncodeCommand(CmdSetViewport{ viewport });
	}
	void CommandBufferVk::SetScissor(const Rect& scissorin)
	{
		EncodeCommand(CmdSetScissor{ scissorin });
	}

    void CommandBufferVk::UseResource(const TextureView& view){
        
    }


	void CommandBufferVk::Commit(const CommitConfig& config)
	{
		owningQueue->Submit(this, config, internalFence);
		vmaSetCurrentFrameIndex(owningQueue->owningDevice->vkallocator, owningQueue->owningDevice->frameIndex++);
		swapchainsToSignal.clear();
	}

	void CommandBufferVk::ExecuteIndirect(const IndirectConfig& config)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(config.indirectBuffer).get(), { .written = false });
		EncodeCommand(CmdExecuteIndirect{ config });
	}
	void CommandBufferVk::DispatchIndirect(const DispatchIndirectConfig& config)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(config.indirectBuffer).get(), { .written = false });
		EncodeCommand(CmdDispatchIndirect{config});
	}
	void CommandBufferVk::BeginRenderDebugMarker(const std::string& label)
	{
#ifndef NDEBUG
		EncodeCommand(CmdBeginDebugMarker{ label });
#endif
	}
	void CommandBufferVk::BeginComputeDebugMarker(const std::string& label)
	{
		BeginRenderDebugMarker(label);
	}
	void CommandBufferVk::EndRenderDebugMarker()
	{
#ifndef NDEBUG
		EncodeCommand(CmdEndDebugMarker{});
#endif
	}
	void CommandBufferVk::EndComputeDebugMarker()
	{
		EndRenderDebugMarker();
	}
	void CommandBufferVk::BlockUntilCompleted()
	{
		vkWaitForFences(owningQueue->owningDevice->device, 1, &internalFence, VK_TRUE, UINT64_MAX);
	}
	void CommandBufferVk::ExecuteIndirectIndexed(const IndirectConfig& config)
	{
		RecordBufferBinding(std::static_pointer_cast<BufferVk>(config.indirectBuffer).get(), { .written = false });
		EncodeCommand(CmdExecuteIndirectIndexed{ config });
	}
	CommandBufferVk::CommandBufferVk(decltype(owningQueue) owningQueue) : owningQueue(owningQueue)
	{
		auto device = owningQueue->owningDevice->device;
		VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = owningQueue->owningDevice->commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
		};
		VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));
		VkFenceCreateInfo fenceInfo{
		  .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		  .flags = VkFenceCreateFlags(0)
		};
		VK_CHECK(vkCreateFence(owningQueue->owningDevice->device, &fenceInfo, nullptr, &internalFence));
	}
	CommandBufferVk::~CommandBufferVk()
	{
		vkWaitForFences(owningQueue->owningDevice->device, 1, &internalFence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(owningQueue->owningDevice->device, internalFence, nullptr);
	}

	void CommandBufferVk::RecordBufferBinding(const BufferVk* buffer, BufferLastUse usage)
	{
		auto recUsage = [&]() {
			activeBuffers[buffer] = usage;
			};

		auto it = activeBuffers.find(buffer);

		if (it == activeBuffers.end()) {
			// record the usage but don't set a barrier
			recUsage();
			return;
		}

		if (!it->second.written) {
			recUsage();
			return;
		}

		// a previous usage might have changed the data, so we need to sync
		auto owningDeviceFamily = buffer->owningDevice->indices.graphicsFamily.value();
		VkBufferMemoryBarrier2 bufferBarrier{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT | VK_ACCESS_2_HOST_WRITE_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_INDEX_READ_BIT | VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_HOST_READ_BIT | VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
				.srcQueueFamilyIndex = owningDeviceFamily,
				.dstQueueFamilyIndex = owningDeviceFamily,
				.buffer = buffer->buffer,
				.offset = 0,
				.size = buffer->getBufferSize()
		};
		barriersToAdd.push_back(bufferBarrier);


		recUsage();
	}

	void CommandBufferVk::RecordTextureBinding(const TextureView texture, TextureLastUse usage, bool recordOnly)
	{
		auto parentVk = static_cast<const TextureVk*>(texture.parent);
		TextureLastUseKey key{parentVk,texture.texture.vk.coveredMips, texture.texture.vk.coveredLayers };

		auto barrierForView = [this, usage, recordOnly](TextureLastUseKey key) {
			auto it = activeTextures.find(key);
			auto needed = usage.lastLayout;
			if (it == activeTextures.end()) {
				activeTextures[key] = {
					.lastLayout = key.texture->nativeFormat,
					.written = false
				};
				it = activeTextures.find(key);
			}

			auto current = (*it).second.lastLayout;
			if (current == needed) {
				return;
			}

			if (recordOnly) {
				return;
			}

			VkImageMemoryBarrier transitionbarrier{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
					.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
					.oldLayout = current,
					.newLayout = needed,
					.image = key.texture->vkImage,
					.subresourceRange = {
					  .aspectMask = key.texture->createdAspectVk,
					  .baseMipLevel = MaskToMipLevel(key.coveredMips),
					  .levelCount = 1,
					  .baseArrayLayer = MaskToLayer(key.coveredLayers),
					  .layerCount = 1,
					}
			};

			// insert the transition barrier
			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,  // srcStageMask	
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // dstStageMask
				0,
				0,
				nullptr,
				0,
				nullptr,
				1, // imageMemoryBarrierCount
				&transitionbarrier // pImageMemoryBarriers
			);

			// update tracker
			(*it).second = {
				.lastLayout = needed,
				.written = usage.written
			};
		};

		constexpr static auto iterateMask = [](auto mask, uint32_t max_index,  auto && func) {
			uint32_t index = 0;
			while (mask > 0 && index < max_index) {
				// get the LSB
				bool LSB = mask & 0b1;
				if (LSB) {
					func(index);
				}
				mask >>= 1;
				index++;
			}
		};

		iterateMask(key.coveredMips,parentVk->createdConfig.mipLevels, [&key = std::as_const(key), parentVk = std::as_const(parentVk), &barrierForView](auto mipLevel) {
			iterateMask(key.coveredLayers, parentVk->createdConfig.arrayLayers, [&key = std::as_const(key), &mipLevel, &barrierForView](auto layerIndex) {
				auto keyCopy = key;
				keyCopy.coveredMips = MakeMipMaskForIndex(mipLevel);
				keyCopy.coveredLayers = MakeLayerMaskForIndex(layerIndex);
				barrierForView(keyCopy);
			});
		});
	}

	template<typename ... Ts>
	struct Overload : Ts ... {
		using Ts::operator() ...;
	};
	template<class... Ts> Overload(Ts...) -> Overload<Ts...>;
	void CommandBufferVk::EncodeQueuedCommands()
	{
		auto visitor = Overload{
			[this](const CmdBeginRendering& arg) mutable {
				ApplyBarriers();

				auto renderPass = std::static_pointer_cast<RenderPassVk>(arg.pass);

				currentRenderPass = renderPass;

				stackarray(attachmentInfos, VkRenderingAttachmentInfoKHR, renderPass->config.attachments.size());

				auto makeAttachmentInfo = [](const RenderPassConfig::AttachmentDesc& attachment, VkImageView imageView) -> VkRenderingAttachmentInfoKHR {
					VkClearValue clearColor = { {{attachment.clearColor[0], attachment.clearColor[1], attachment.clearColor[2], attachment.clearColor[3]}} };

					return {
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
						.imageView = imageView,
						.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
						.loadOp = RGL2LoadOp(attachment.loadOp),
						.storeOp = RGL2StoreOp(attachment.storeOp),
						.clearValue = clearColor,
					};
				};


				uint32_t i = 0;
				for (const auto& attachment : renderPass->config.attachments) {

					attachmentInfos[i] = makeAttachmentInfo(attachment, renderPass->textures[i].texture.vk.view);

					// the swapchain image may be in the wrong state (present state vs write state) so it needs to be transitioned
					auto castedImage = static_cast<const TextureVk*>(renderPass->textures[i].parent);

					if (castedImage->owningSwapchain) {
						swapchainsToSignal.insert(castedImage->owningSwapchain);
						swapchainImages.insert(castedImage);
					}

					i++;
				}

				// repeat for depth stencil attachment
				RGL::Dimension texSize{};
				if (renderPass->textures.size() > 0) {
					texSize = renderPass->textures[0].viewSize;
				}
				else if (renderPass->depthTexture) {
					texSize = renderPass->depthTexture->viewSize;
				}
				else {
					FatalError("No rendertargets are bound, cannot get texture size for beginRendering");
				}

				VkRenderingAttachmentInfoKHR* depthAttachmentinfo = nullptr;
				VkRenderingAttachmentInfoKHR depthAttachmentInfoData{};

				if (renderPass->config.depthAttachment.has_value()) {
					auto& da = renderPass->config.depthAttachment.value();
					depthAttachmentInfoData = makeAttachmentInfo(da, renderPass->depthTexture->texture.vk.view);
					depthAttachmentinfo = &depthAttachmentInfoData;
				}

				VkRenderingAttachmentInfoKHR* stencilAttachmentInfo = nullptr;
				VkRenderingAttachmentInfoKHR stencilAttachmentInfoData{};

				if (renderPass->config.stencilAttachment.has_value()) {
					auto& sa = renderPass->config.stencilAttachment.value();
					stencilAttachmentInfoData = makeAttachmentInfo(sa, renderPass->depthTexture->texture.vk.view);
					stencilAttachmentInfo = &stencilAttachmentInfoData;
				}


				const VkRenderingInfoKHR render_info{
					.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
					.renderArea = {
						.offset = {0,0},
						.extent = VkExtent2D{.width = texSize.width, .height = texSize.height},
					},
					.layerCount = 1,
					.colorAttachmentCount = static_cast<uint32_t>(renderPass->config.attachments.size()),
					.pColorAttachments = attachmentInfos,
					.pDepthAttachment = depthAttachmentinfo,
					.pStencilAttachment = stencilAttachmentInfo
				};


				vkCmdBeginRendering(commandBuffer, &render_info);
			},
			[this](const CmdSetVertexBuffer& arg) {
				auto vkbuffer = std::static_pointer_cast<BufferVk>(arg.buffer);
				VkBuffer vertexBuffers[] = { vkbuffer->buffer };
				VkDeviceSize offsets[] = { arg.bindingInfo.offsetIntoBuffer * vkbuffer->stride };
				vkCmdBindVertexBuffers(commandBuffer, arg.bindingInfo.bindingPosition, std::size(vertexBuffers), vertexBuffers, offsets);
			},
			[this](const CmdSetIndexBuffer& arg) {
				const auto casted = std::static_pointer_cast<BufferVk>(arg.buffer);
				const auto size_type = casted->stride == sizeof(uint16_t) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
				vkCmdBindIndexBuffer(commandBuffer, casted->buffer, 0, size_type);
			},
			[this](const CmdSetSampler& arg) {
				auto& sampler = arg.sampler;
				auto index = arg.index;
				VkDescriptorImageInfo imginfo{
							.sampler = std::static_pointer_cast<SamplerVk>(sampler)->sampler,
							.imageView = VK_NULL_HANDLE,
							.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				};
				VkWriteDescriptorSet writeinfo{
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.dstSet = VK_NULL_HANDLE,
						.dstBinding = index,
						.dstArrayElement = 0,
						.descriptorCount = 1,
						.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
						.pImageInfo = &imginfo,
						.pBufferInfo = nullptr,
						.pTexelBufferView = nullptr
				};
				owningQueue->owningDevice->vkCmdPushDescriptorSetKHR(
					commandBuffer,
					arg.isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
					arg.isCompute ? currentComputePipeline->pipelineLayout->layout : currentRenderPipeline->pipelineLayout->layout,
					0,
					1,
					&writeinfo
				);
			},
			[this](const CmdSetTexture& arg) {
				auto texture = arg.texture;
				auto index = arg.index;
				auto key = TextureLastUseKey{ static_cast<const TextureVk*>(texture.parent), texture.texture.vk.coveredMips, texture.texture.vk.coveredLayers };
				if (key.coveredMips == ALL_MIPS) {
					key.coveredMips = MakeMipMaskForIndex(0);
				}
				if (key.coveredLayers == ALL_LAYERS) {
					key.coveredLayers = MakeLayerMaskForIndex(0);
				}

				auto it = activeTextures.find(key);
				auto layout = key.texture->nativeFormat;
				if (it != activeTextures.end()) {
					layout = it->second.lastLayout;
				}
				bool isCompute = currentRenderPipeline ? false : true;
				auto activeLayout = isCompute ? currentComputePipeline->pipelineLayout : currentRenderPipeline->pipelineLayout;

				VkDescriptorImageInfo imginfo{
							.sampler = VK_NULL_HANDLE,
							.imageView = texture.texture.vk.view,
							.imageLayout = layout,
				};
				VkWriteDescriptorSet writeinfo{
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.dstSet = VK_NULL_HANDLE,
						.dstBinding = index,
						.dstArrayElement = 0,
						.descriptorCount = 1,
						.descriptorType = activeLayout->bindingDescriptorTypes.at(index),
						.pImageInfo = &imginfo,
						.pBufferInfo = nullptr,
						.pTexelBufferView = nullptr
				};


				owningQueue->owningDevice->vkCmdPushDescriptorSetKHR(
					commandBuffer,
					isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
					activeLayout->layout,
					0,
					1,
					&writeinfo
				);

				if (key.texture->owningSwapchain) {
					swapchainsToSignal.insert(key.texture->owningSwapchain);
					swapchainImages.insert(key.texture);
				}
			},
			[this](const CmdBindlessSetTexture& arg) {
				bool isCompute = currentRenderPipeline ? false : true;
				auto activeLayout = isCompute ? currentComputePipeline->pipelineLayout : currentRenderPipeline->pipelineLayout;

				vkCmdBindDescriptorSets(commandBuffer,
					isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
					activeLayout->layout,
					1,
					1,
					& arg.set,
					0,
					nullptr
				);
			},
			[this](const CmdDraw& arg) {
				vkCmdDraw(commandBuffer, arg.nVertices, arg.config.nInstances, arg.config.startVertex, arg.config.firstInstance);
			},
			[this](const CmdDrawIndexed& arg) {
				vkCmdDrawIndexed(commandBuffer, arg.nIndices, arg.config.nInstances, arg.config.firstIndex, arg.config.startVertex, arg.config.firstInstance);
			},
			[this](const CmdBindBuffer& arg) {
				auto buffer = arg.buffer;
				auto offsetIntoBuffer = arg.offsetIntoBuffer;
				auto bindingOffset = arg.bindingOffset;
				auto bindPoint = arg.bindPoint;
				auto vkbuffer = std::static_pointer_cast<BufferVk>(buffer);
				VkDescriptorBufferInfo bufferInfo{
					.buffer = vkbuffer->buffer,
					.offset = offsetIntoBuffer * vkbuffer->stride,
					.range = VK_WHOLE_SIZE,
				};
				VkWriteDescriptorSet writeinfo{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = VK_NULL_HANDLE,	// we push descriptors instead
					.dstBinding = bindingOffset,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pImageInfo = nullptr,
					.pBufferInfo = &bufferInfo,
					.pTexelBufferView = nullptr
				};
				owningQueue->owningDevice->vkCmdPushDescriptorSetKHR(
					commandBuffer,
					bindPoint,
					(bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE ? currentComputePipeline->pipelineLayout->layout : currentRenderPipeline->pipelineLayout->layout),
					0,
					1,
					&writeinfo
				);
			},
			[this](const CmdExecuteIndirectIndexed& arg) {
				auto& config = arg.config;
				const auto buffer = std::static_pointer_cast<BufferVk>(config.indirectBuffer);
				vkCmdDrawIndexedIndirect(commandBuffer,
					buffer->buffer,
					config.offsetIntoBuffer,
					config.nDraws,
					sizeof(IndirectIndexedCommand)
				);
			},
			[this](const CmdExecuteIndirect& arg) {
				auto& config = arg.config;
				const auto buffer = std::static_pointer_cast<BufferVk>(config.indirectBuffer);
				vkCmdDrawIndirect(commandBuffer,
					buffer->buffer,
					config.offsetIntoBuffer,
					config.nDraws,
					sizeof(IndirectCommand)
				);
			},
			[this](const CmdDispatchIndirect& arg) {
				auto& config = arg.config;
				const auto buffer = std::static_pointer_cast<BufferVk>(config.indirectBuffer);
				vkCmdDispatchIndirect(commandBuffer,
					buffer->buffer,
					config.offsetIntoBuffer
				);
			},
			[this](const CmdSetPushConstantData& arg) {
				auto data = arg.data;

				if (currentRenderPipeline) {
					const auto& flagsStore = currentRenderPipeline->pipelineLayout->pushConstantBindingStageFlags;

					auto stages = flagsStore.contains(arg.offset) ? flagsStore.at(arg.offset) : VK_SHADER_STAGE_COMPUTE_BIT;

					auto layout = currentRenderPipeline->pipelineLayout->layout;

					vkCmdPushConstants(commandBuffer, layout, stages, arg.offset, arg.size, data);
				}
				else {
					const auto& flagsStore = currentComputePipeline->pipelineLayout->pushConstantBindingStageFlags;

					auto stages = VK_SHADER_STAGE_COMPUTE_BIT;

					auto layout = currentComputePipeline->pipelineLayout->layout;

					vkCmdPushConstants(commandBuffer, layout, stages, arg.offset, arg.size, data);

					assert(currentComputePipeline != nullptr);
				}


			},
			[this](const CmdBindRenderPipeline& arg) {
				auto pipeline = std::static_pointer_cast<RenderPipelineVk>(arg.generic_pipeline);
				// drawing commands
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphicsPipeline);
				currentRenderPipeline = pipeline;
			},
			[this](const CmdBeginDebugMarker& arg) {
				VkDebugUtilsLabelEXT markerInfo = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pLabelName = arg.label.c_str()
					};
				owningQueue->owningDevice->rgl_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &markerInfo);
			},
			[this](const CmdEndDebugMarker&) {
				owningQueue->owningDevice->rgl_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
			},
			[this](const CmdBeginCompute& arg) {
				ApplyBarriers();
				currentComputePipeline = std::static_pointer_cast<ComputePipelineVk>(arg.inPipeline);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, currentComputePipeline->computePipeline);
			},
			[this](const CmdEndCompute&) {
				currentComputePipeline = nullptr;
			},
			[this](const CmdDispatch& arg) {
				vkCmdDispatch(commandBuffer, arg.threadsX, arg.threadsY, arg.threadsZ);
			},
			[this](const CmdCopyTextureToBuffer& arg) {
				auto castedDest = std::static_pointer_cast<BufferVk>(arg.destBuffer);
				auto castedImage = static_cast<const TextureVk*>(arg.sourceTexture.parent);

				VkBufferImageCopy region{
					.bufferOffset = 0,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = 0,
						.baseArrayLayer = 0,
						.layerCount = 1
					},
					.imageOffset = {
						.x = arg.sourceRect.offset[0],
						.y = arg.sourceRect.offset[1],
						.z = 0
					},
					.imageExtent = {
						.width = arg.sourceRect.extent[0],
						.height = arg.sourceRect.extent[1],
						.depth = 1
					}
				};

				vkCmdCopyImageToBuffer(commandBuffer, castedImage->vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, castedDest->buffer, 1, &region);
			},
			[this](const CmdCopyTextureToTexture& arg) {
				auto src = static_cast<const TextureVk*>(arg.from.texture.parent);
				auto dst = static_cast<const TextureVk*>(arg.to.texture.parent);

				auto& srcLayout = activeTextures.at(TextureLastUseKey{src, arg.from.texture.texture.vk.coveredMips, arg.from.texture.texture.vk.coveredLayers });
				auto& dstLayout = activeTextures.at(TextureLastUseKey{ dst, arg.to.texture.texture.vk.coveredMips, arg.to.texture.texture.vk.coveredLayers });

				auto dim = src->GetSize();
				VkImageCopy2 region{
					.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
					.pNext = nullptr,
					.srcSubresource = {
						.aspectMask = src->createdAspectVk,
						.mipLevel = arg.fromMip,
						.baseArrayLayer = arg.fromLayer,
						.layerCount = 1
					},
					.srcOffset = {0,0,0},
					.dstSubresource = {
						.aspectMask = dst->createdAspectVk,
						.mipLevel = arg.toMip,
						.baseArrayLayer = arg.toLayer,
						.layerCount = 1
					},
					.dstOffset = {0,0,0},
					.extent = {dim.width, dim.height ,1}
				};
				VkCopyImageInfo2 copyInfo{
					.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
					.pNext = nullptr,
					.srcImage = src->vkImage,
					.srcImageLayout = srcLayout.lastLayout,
					.dstImage = dst->vkImage,
					.dstImageLayout = dstLayout.lastLayout,
					.regionCount = 1,
					.pRegions = &region,
				};
				vkCmdCopyImage2(commandBuffer, &copyInfo);
			},
			[this](const CmdSetViewport& arg) {
				auto& viewport = arg.viewport;
				VkViewport vp{
					.x = viewport.x,
					.y = viewport.height - viewport.y,
					.width = viewport.width,
					.height = -viewport.height, // make Vulkan a Y-up system
					.minDepth = viewport.minDepth,
					.maxDepth = viewport.maxDepth
				};
				vkCmdSetViewport(commandBuffer, 0, 1, &vp);
			},
			[this](const CmdSetScissor& arg) {
				auto& scissorin = arg.scissor;
				VkRect2D scissor{
				.offset = {scissorin.offset[0], scissorin.offset[1]},
				.extent = {scissorin.extent[0], scissorin.extent[1]},
				};
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
			},
			[this](const CmdCopyBufferToBuffer& arg) {
				VkBufferCopy bufferCopyData{
				   .srcOffset = arg.from.offset,
				   .dstOffset = arg.to.offset,
				   .size = arg.size
				};
				auto fromBuffer = std::static_pointer_cast<BufferVk>(arg.from.buffer);
				auto toBuffer = std::static_pointer_cast<BufferVk>(arg.to.buffer);
				vkCmdCopyBuffer(commandBuffer, fromBuffer->buffer, toBuffer->buffer, 1, &bufferCopyData);
			},
			[this](const CmdCopyBuffertoTexture& arg) {
				VkBufferImageCopy region{};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = arg.arrayLayer;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = {
					arg.destLoc.extent[0],
					arg.destLoc.extent[0],
					1
				};

				vkCmdCopyBufferToImage(
					commandBuffer,
					std::static_pointer_cast<BufferVk>(arg.srcBuffer)->buffer,
					static_cast<const TextureVk*>(arg.destTexture.parent)->vkImage,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			},
		};

		for (const auto& item : renderCommands) {
			std::visit(visitor, item);
		}

		renderCommands.clear();
	}
	bool CommandBufferVk::IsBufferSlotWritable(uint32_t slot)
	{
		auto findWritten = [&](auto&& bindingStore, bool& out) {
			if (!bindingStore) {
				out = false;
				return;
			}

			auto it = bindingStore->find(slot);
			if (it != bindingStore->end()) {
				out = it->second.writable;
			}
			else {
				out = false;
			}
			};

		if (currentRenderPipeline) {
			bool vsWritable = false, fsWritable = false;

			findWritten(currentRenderPipeline->vsBufferBindings, vsWritable);
			findWritten(currentRenderPipeline->fsBufferBindings, fsWritable);

			return vsWritable || fsWritable;
		}
		else if (currentComputePipeline) {
			bool isWritable = false;
			findWritten(currentComputePipeline->bufferBindings, isWritable);
			return isWritable;
		}
		else {
			Assert(false, "Invalid state!");
		}

	}
	void CommandBufferVk::ApplyBarriers()
	{
		if (barriersToAdd.size() == 0) {
			return;
		}

		VkDependencyInfo depInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = nullptr,
			.bufferMemoryBarrierCount = uint32_t(barriersToAdd.size()),
			.pBufferMemoryBarriers = barriersToAdd.data(),
			.imageMemoryBarrierCount = 0,
			.pImageMemoryBarriers = nullptr
		};
		vkCmdPipelineBarrier2(
			commandBuffer,
			&depInfo
		);
		barriersToAdd.clear();
	}
}

#endif
