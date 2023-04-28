#pragma once
#include <RGL/Types.hpp>
#include <RGL/Synchronization.hpp>
#include <vulkan/vulkan.h>
#include <memory>

namespace RGL {
	struct DeviceVk;

	struct FenceVk : public IFence {
		VkFence fence = VK_NULL_HANDLE;
		const std::shared_ptr<DeviceVk> owningDevice;
		FenceVk(decltype(owningDevice) device, bool preSignaled);
		void Wait() final;
		void Reset() final;
		void Signal() final;
		virtual ~FenceVk();
	};
}