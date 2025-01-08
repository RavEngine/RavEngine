#if RGL_VK_AVAILABLE
#include "VkBuffer.hpp"
#include "VkDevice.hpp"
#include "RGLVk.hpp"
#include <vk_mem_alloc.h>
#include <cstring>

namespace RGL {

    VkBufferUsageFlags rgl2vkbufferflags(RGL::BufferConfig::Type type) {
        VkBufferUsageFlags flags = 0;
        if (type.IndexBuffer) {
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (type.IndirectBuffer) {
            flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }
        if (type.StorageBuffer) {
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        if (type.UniformBuffer) {
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if (type.VertexBuffer) {
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        return flags;
    }

	BufferVk::BufferVk(decltype(owningDevice) owningDevice, const BufferConfig& config) : owningDevice(owningDevice), accessType(config.access) {
        
        VkMemoryPropertyFlags memprop = 0;
        switch (config.access) {
        case decltype(config.access)::Private:
            memprop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case decltype(config.access)::Shared:
            memprop = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        default:
            FatalError("Unsupported access");
        }

        auto usage = rgl2vkbufferflags(config.type);
        if (config.options.TransferDestination || config.access == decltype(config.access)::Private) {
            usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        if (config.options.Transfersource) {
            usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        allocation = createBuffer(owningDevice.get(), config.nElements * config.stride, usage, memprop, buffer);

        if (config.options.debugName) {
            owningDevice->SetDebugNameForResource((void*)buffer, VK_OBJECT_TYPE_BUFFER, config.options.debugName);
#ifndef NDEBUG
            debugName = config.options.debugName;
#endif
        }

        mappedMemory.size = config.nElements * config.stride;
        stride = config.stride;

        if (config.type.StorageBuffer) {
            // make a descriptor for the global descriptor buffer and put it in the buffer
            globalDescriptorIndex = owningDevice->globalBufferDescriptorFreeList.Allocate();

            VkDescriptorBufferInfo bufferInfo{
                .buffer = buffer,
                .offset = 0,
                .range = config.nElements * config.stride
            };

            VkWriteDescriptorSet bindlessDescriptorWrite{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = owningDevice->globalBufferDescriptorSet,
                .dstBinding = 0,							// bindless is always at binding 0 set N
                .dstArrayElement = globalDescriptorIndex,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &bufferInfo,
                .pTexelBufferView = nullptr
            };

            vkUpdateDescriptorSets(owningDevice->device, 1, &bindlessDescriptorWrite, 0, nullptr);
        }
	}

    BufferVk::~BufferVk() {
        if (mappedMemory.data != nullptr) {
            UnmapMemory();
        }
        vkDestroyBuffer(owningDevice->device, buffer, nullptr);
        vmaFreeMemory(owningDevice->vkallocator, allocation);

        if (globalDescriptorIndex != unallocated) {
            owningDevice->globalBufferDescriptorFreeList.Deallocate(globalDescriptorIndex);
        }
    }

    void BufferVk::SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset) {
        if (accessType == RGL::BufferAccess::Shared) {
            UpdateBufferData(data, offset);
            UnmapMemory();
        }
        else {
            // copy to buffer from staging buffer
            VkBuffer tmpBuffer;
            auto tmpBufferAlloc = createBuffer(owningDevice.get(), data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, tmpBuffer);

            // copy to staging buffer
            void* mappedPtr = nullptr;
            vmaMapMemory(owningDevice->vkallocator, tmpBufferAlloc, &mappedPtr);
            std::memcpy(mappedPtr, data.data(), data.size());
            vmaUnmapMemory(owningDevice->vkallocator, tmpBufferAlloc);

            // copy staging to real
            VkFence uploadFence;
            auto commandBuffer = beginSingleTimeCommands(owningDevice->device, owningDevice->commandPool);

            VkBufferCopy bufferCopyData{
                .srcOffset = 0,
                .dstOffset = offset,
                .size = data.size()
            };

            vkCmdCopyBuffer(commandBuffer, tmpBuffer, buffer, 1, &bufferCopyData);
            endSingleTimeCommands(commandBuffer, owningDevice->presentQueue, owningDevice->device, owningDevice->commandPool);

            // free
            vkDestroyBuffer(owningDevice->device, tmpBuffer, nullptr);
            vmaFreeMemory(owningDevice->vkallocator, tmpBufferAlloc);
        }
       
    }

    decltype(BufferConfig::nElements) BufferVk::getBufferSize() const
    {
        return mappedMemory.size;
    }

    void* BufferVk::GetMappedDataPtr()
    {
        return mappedMemory.data;
    }

    uint32_t BufferVk::GetReadonlyBindlessGPUHandle() const
    {
        return globalDescriptorIndex;
    }

    uint32_t BufferVk::GetReadwriteBindlessGPUHandle() const
    {
        return globalDescriptorIndex;
    }

    void BufferVk::MapMemory() {
        vmaMapMemory(owningDevice->vkallocator, allocation, &mappedMemory.data);
    }
    void BufferVk::UnmapMemory() {
        vmaUnmapMemory(owningDevice->vkallocator, allocation);
        mappedMemory.data = nullptr;
    }

    void BufferVk::UpdateBufferData(untyped_span data, decltype(BufferConfig::nElements) offset) {
        if (!mappedMemory.data) {
            MapMemory();
        }
        Assert(data.size() + offset <= mappedMemory.size, "Attempting to write more data than the buffer can hold");
        memcpy(static_cast<std::byte*>(mappedMemory.data) + offset, data.data(), data.size());
    }

    void BufferVk::SignalRangeChanged(const Range & range){
        
    }
}

#endif
