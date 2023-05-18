#include "RenderEngine.hpp"
#include <RGL/Span.hpp>
#include <mutex>
#include <RGL/RGL.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/Device.hpp>

namespace RavEngine {
	RenderEngine::MeshRange RenderEngine::AllocateMesh(std::span<VertexNormalUV> vertices, std::span<uint32_t> indices)
	{
        std::lock_guard{allocationLock};
		auto const vertexBytes = std::as_bytes(vertices);
		auto const indexBytes = std::as_bytes( indices );

	
		/**
		Find a Range that can fit the current allocation. If one does not exist, returns -1 (aka uint max)
		*/
		auto findPlacement = [](uint32_t requestedSize, const allocation_freelist_t& freeList) {
			uint32_t bestRangeIndex = -1;
			for (uint32_t i = 0; i < freeList.size(); i++) {
				auto& nextRange = freeList[i];
				if (requestedSize <= freeList[bestRangeIndex].count) {
					bestRangeIndex = i;
					break;
				}
			}
			return bestRangeIndex;
		};

		auto getAllocationLocation = [&findPlacement](uint32_t& allocation, uint32_t size, const uint32_t& currentSize, const allocation_freelist_t& freeList, auto realloc_fn) {
			allocation = -1;
			do {
				allocation = findPlacement(size, freeList);
				if (allocation == -1) {
					// resize to fit
					realloc_fn(currentSize + size);
				}

			} while (allocation != -1);
		};

		// figure out where to put the new data, resizing the buffer as needed
		uint32_t vertexAllocation = -1, indexAllocation = -1;
		getAllocationLocation(vertexAllocation, vertexBytes.size(), currentVertexSize, vertexFreeList,[this](uint32_t newSize) {ReallocateVertexAllocationToSize(newSize); });
		getAllocationLocation(indexAllocation, indexBytes.size(), currentIndexSize, indexFreeList,  [this](uint32_t newSize) {ReallocateIndexAllocationToSize(newSize); });

		// now we have the location to place the vertex and index data in the buffer
		// these numbers are stable because, if the buffer resized, then the only place it could be stored is at the end.
		// if the new data fits, then the buffer was not resized, so the indices are stable.

		auto consumeRange = [](uint32_t allocation, uint32_t allocatedSize, allocation_freelist_t& freeList, allocation_allocatedlist_t& allocatedList) {
			auto& rangeToUpdate = freeList.at(allocation);
			// if it fits exactly, delete it
			if (rangeToUpdate.count == allocatedSize) {
				freeList.erase(freeList.begin() + allocation);
			}
			else {
				// otherwise, modify it to represent the new shrunken size
				rangeToUpdate.start += allocatedSize;
			}
			// mark as allocated (copy into)
			allocatedList.push_back(rangeToUpdate);
			return rangeToUpdate;
		};

		// mark the ranges as consumed
		auto vertexPlacement = consumeRange(vertexAllocation, vertexBytes.size(), vertexFreeList, vertexAllocatedList);
		auto indexPlacement = consumeRange(indexAllocation, indexBytes.size(), indexFreeList, indexAllocatedList);

		// upload buffer data
		sharedVertexBuffer->UpdateBufferData(
			{ vertexBytes.data(), vertexBytes.size() }, vertexPlacement.start
		);
		sharedIndexBuffer->UpdateBufferData(
			{ indexBytes.data(), indexBytes.size() }, indexPlacement.start
		);
		
	}
	void RenderEngine::DeallocateMesh(const MeshRange& range)
	{
        std::lock_guard{allocationLock};
		
		auto deallocateData = [](Range range, allocation_allocatedlist_t& allocatedList, allocation_freelist_t& freeList) {
			freeList.push_back(range);

			uint32_t foundRangeIndex = 0;
			for (; foundRangeIndex < allocatedList.size(); foundRangeIndex++) {
				const auto& nextRange = allocatedList[foundRangeIndex];
				if (nextRange.start >= range.start && nextRange.count >= range.count) {
					break;
				}
			}

			// 3 options:
			// 1: input range coincides with the beginning of the found range but is smaller
			// 2: input range splits the found range
			// 3: input range coincides with the end of the found range but starts 
			
			// on pushing a new range onto the freelist:
				// does this range encompass the final range in the freelist? if so, only update that one

		};

		deallocateData(range.vertRange, vertexAllocatedList, vertexFreeList);
		deallocateData(range.indexRange, indexAllocatedList, indexFreeList);
	}

	void RavEngine::RenderEngine::ReallocateVertexAllocationToSize(uint32_t newSize)
	{
		//TODO: compaction pass goes here

		sharedVertexBuffer = device->CreateBuffer({
			newSize,
			{.VertexBuffer = true},
			sizeof(VertexNormalUV),
			RGL::BufferAccess::Private
		});
		currentVertexSize = newSize;
		
	}
	void RenderEngine::ReallocateIndexAllocationToSize(uint32_t newSize)
	{
		sharedIndexBuffer = device->CreateBuffer({
			newSize,
			{.IndexBuffer = true},
			sizeof(uint32_t),
			RGL::BufferAccess::Private
		});
		currentIndexSize = newSize;
	}
}
