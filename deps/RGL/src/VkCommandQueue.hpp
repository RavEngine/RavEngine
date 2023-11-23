#pragma once
#include <RGL/Types.hpp>
#include <RGL/CommandQueue.hpp>
#include "VkSynchronization.hpp"
#include <vulkan/vulkan.h>
#include <span>

namespace RGL {
	struct DeviceVk;
	struct ICommandBuffer;
	struct CommandBufferVk;
	struct CommitConfig;
	
	struct CommandQueueVk : public ICommandQueue, public std::enable_shared_from_this<CommandQueueVk> {
		const std::shared_ptr<DeviceVk> owningDevice;
		VkQueue queue;
		CommandQueueVk(decltype(owningDevice) device);

		// call by commandbuffer::commit
		void Submit(CommandBufferVk*, const CommitConfig&, VkFence internalFence);
		
		// ICommandQueue
        RGLCommandBufferPtr CreateCommandBuffer() final;

		void WaitUntilCompleted() final;

		QueueData GetQueueData() final;
	};

}
