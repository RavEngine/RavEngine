#pragma once
#include <RGL/Types.hpp>
#include <RGL/Buffer.hpp>
#include <memory>
#include <span>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#ifndef NDEBUG
#include <string>
#endif

namespace RGL {
	struct DeviceVk;

	struct BufferVk : public IBuffer {
		const std::shared_ptr<DeviceVk> owningDevice;
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		const RGL::BufferAccess accessType;

		MutableSpan mappedMemory;
		size_t stride = 0;

		BufferVk(decltype(owningDevice), const BufferConfig&);
		virtual ~BufferVk();

		void MapMemory() final;
		void UnmapMemory() final;
		void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) final;
		void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) final;
		decltype(BufferConfig::nElements) getBufferSize() const final;
        
        void SignalRangeChanged(const Range&) final;
        
		void* GetMappedDataPtr() final;

#ifndef NDEBUG
		std::string debugName;
#endif
	};
}
