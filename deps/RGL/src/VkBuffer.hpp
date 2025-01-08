#pragma once
#include <RGL/Types.hpp>
#include <RGL/Buffer.hpp>
#include <memory>
#include <span>
#include <volk.h>
#ifndef NDEBUG
#include <string>
#endif

struct VmaAllocation_T;

namespace RGL {
	struct DeviceVk;

	struct BufferVk : public IBuffer {
		const std::shared_ptr<DeviceVk> owningDevice;
		VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation_T* allocation = VK_NULL_HANDLE;
		const RGL::BufferAccess accessType;

		MutableSpan mappedMemory;
		size_t stride = 0;

		constexpr static uint32_t unallocated = std::numeric_limits<uint32_t>::max();
		uint32_t globalDescriptorIndex = unallocated;

		BufferVk(decltype(owningDevice), const BufferConfig&);
		virtual ~BufferVk();

		void MapMemory() final;
		void UnmapMemory() final;
		void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) final;
		void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) final;
		decltype(BufferConfig::nElements) getBufferSize() const final;
        
        void SignalRangeChanged(const Range&) final;
        
		void* GetMappedDataPtr() final;

		uint32_t GetReadonlyBindlessGPUHandle() const;
		uint32_t GetReadwriteBindlessGPUHandle() const;

#ifndef NDEBUG
		std::string debugName;
#endif
	};
}
