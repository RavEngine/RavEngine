#if !RVE_SERVER
#include "RenderEngine.hpp"
#include <RGL/Span.hpp>
#include <mutex>
#include <RGL/RGL.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/Device.hpp>
#include <RGL/CommandBuffer.hpp>
#include "Debug.hpp"

namespace RavEngine {
	MeshRange RenderEngine::AllocateMesh(const MeshPartView& mesh)
	{
        std::lock_guard mtx{allocationLock};
	
		using alloc_iterator_t = allocation_allocatedlist_t::iterator;
		using freelist_iterator_t = allocation_freelist_t::iterator;

		/**
		Find a Range that can fit the current allocation. If one does not exist, returns -1 (aka uint max)
		@return an interator into the freelist.
		*/
		auto findPlacement = [](uint32_t requestedSize, allocation_freelist_t& freeList) {
			auto bestRangeIndex = freeList.begin();
			for (auto it = freeList.begin(); it != freeList.end(); it++) {
				auto& nextRange = *it;
				if (requestedSize <= (*bestRangeIndex).count) {
					return it;
				}
			}
			return freeList.end();
		};

		/**
		* Given a byte size, find a possible block, resizing the underlying memory if necessary.
		* @returns an iterator into the freelist
		*/
		auto getAllocationLocation = [&findPlacement](uint32_t size, const uint32_t& currentSize, allocation_freelist_t& freeList, auto realloc_fn) {
			auto allocation = freeList.end();
			do {
				allocation = findPlacement(size, freeList);
				if (allocation == freeList.end()) {
					// resize to fit
					realloc_fn(currentSize + size);
				}

			} while (allocation == freeList.end());
			return allocation;
		};

		// figure out where to put the new data, resizing the buffer as needed
		auto vertexAllocation = getAllocationLocation(mesh.NumVerts(), currentVertexSize, vertexFreeList, [this](uint32_t newSize) {ReallocateVertexAllocationToSize(newSize); });
		auto indexAllocation = getAllocationLocation(mesh.indices.size(), currentIndexSize, indexFreeList,  [this](uint32_t newSize) {ReallocateIndexAllocationToSize(newSize); });

		// now we have the location to place the vertex and index data in the buffer
		// these numbers are stable because, if the buffer resized, then the only place it could be stored is at the end.
		// if the new data fits, then the buffer was not resized, so the indices are stable.

		auto consumeRange = [](alloc_iterator_t allocation, uint32_t allocatedSize, allocation_freelist_t& freeList, allocation_allocatedlist_t& allocatedList) {
			// construct a range to match the current allocation
			auto range = *allocation;
			range.count = allocatedSize;
			// mark as allocated (copy into)
			allocatedList.push_back(range);

			// update the freelist
			auto& rangeToUpdate = *allocation;
			// if it fits exactly, delete it
			if (rangeToUpdate.count == allocatedSize) {
				freeList.erase(allocation);
			}
			else {
				// otherwise, modify it to represent the new shrunken size
				// we allocate aligned with the start of the range
				rangeToUpdate.start += allocatedSize;
				rangeToUpdate.count -= allocatedSize;
			}
			
			return --allocatedList.end();
		};

		// mark the ranges as consumed
		auto vertexPlacement = consumeRange(vertexAllocation, mesh.NumVerts(), vertexFreeList, vertexAllocatedList);
		auto indexPlacement = consumeRange(indexAllocation, mesh.indices.size(), indexFreeList, indexAllocatedList);

		MeshRange range{ vertexPlacement, indexPlacement };

		// upload buffer data
		sharedPositionBuffer->SetBufferData({ mesh.positions.data(), mesh.positions.size_bytes() }, range.getPositionByteStart());
		sharedNormalBuffer->SetBufferData({ mesh.normals.data(), mesh.normals.size_bytes() }, range.getNormalByteStart());
		sharedTangentBuffer->SetBufferData({ mesh.tangents.data(), mesh.tangents.size_bytes() }, range.getTangentByteStart());
		sharedBitangentBuffer->SetBufferData({ mesh.bitangents.data(), mesh.bitangents.size_bytes() }, range.getBitangentByteStart());
		sharedUV0Buffer->SetBufferData({ mesh.uv0.data(), mesh.uv0.size_bytes() }, range.getUVByteStart());
		if (mesh.lightmapUVs.size() > 0) {
			sharedLightmapUVBuffer->SetBufferData({ mesh.lightmapUVs.data(), mesh.lightmapUVs.size_bytes() }, range.getUVByteStart());
		}

		sharedIndexBuffer->SetBufferData(
			{ mesh.indices.data(), mesh.indices.size_bytes() }, range.getIndexRangeByteStart()
		);
		
		return range;
	}
	void RenderEngine::DeallocateMesh(const MeshRange& range)
	{
        std::lock_guard mtx{allocationLock};
		
		auto deallocateData = [](Range range, allocation_allocatedlist_t& allocatedList, allocation_freelist_t& freeList) {

			Range foundRange;
			for (auto it = allocatedList.begin(); it != allocatedList.end(); it++) {
				const auto& nextRange = *it;
				if (nextRange.start >= range.start && nextRange.count >= range.count) {
					foundRange = nextRange;
					allocatedList.erase(it);
					break;
				}
			}

			// xxxxx------xxxxx --> ----------xxxxx --> ----------------

			//does this range border any other ranges? If so, don't push a new range onto the freelist, isntead merge the existing ranges
			bool overlapFound = false;
			for (auto& range : freeList) {
				// the found range overlaps with a range to the left of it
				if (range.start + range.count == foundRange.start) {
					range.count += foundRange.count;
					overlapFound = true;
				}
				// the found range overlaps with a range to the right of it
				if (foundRange.start + foundRange.count == range.start) {
					range.start -= foundRange.count;
					range.count += foundRange.count;
					overlapFound = true;
				}
			}
			if (!overlapFound) {
				freeList.push_back(range);
			}

		};
		if (range.getVertRange().getNodePointer() != nullptr) {
			deallocateData(*range.getVertRange(), vertexAllocatedList, vertexFreeList);
		}
		if (range.getIndexRange().getNodePointer() != nullptr) {
			deallocateData(*range.getIndexRange(), indexAllocatedList, indexFreeList);
		}
	}

	uint32_t RenderEngine::WriteTransient(RGL::untyped_span data)
	{
		auto start = transientOffset;

		if (start + data.size() > transientSizeBytes) {
			Debug::Fatal("Not enough space left in transient buffer");
		}

		std::memcpy((char*)(transientStagingBuffer->GetMappedDataPtr()) + start,data.data(),data.size());

		transientOffset += data.size();
		transientOffset = closest_multiple_of<decltype(transientOffset)>(transientOffset, 16);	// vulkan requires this

		return start;
	}

	void RavEngine::RenderEngine::ReallocateVertexAllocationToSize(uint32_t newSize)
	{
		ReallocateGeneric(sharedPositionBuffer, currentVertexSize, newSize, vertexAllocatedList, vertexFreeList, sizeof(VertexPosition_t), { .StorageBuffer = true, .VertexBuffer = true }, lastResizeFrameVB, "Shared Position Buffer");
		ReallocateGeneric(sharedNormalBuffer, currentVertexSize, newSize, vertexAllocatedList, vertexFreeList, sizeof(VertexNormal_t), { .StorageBuffer = true, .VertexBuffer = true }, lastResizeFrameVB, "Shared Normal Buffer");
		ReallocateGeneric(sharedTangentBuffer, currentVertexSize, newSize, vertexAllocatedList, vertexFreeList, sizeof(VertexTangent_t), { .StorageBuffer = true, .VertexBuffer = true }, lastResizeFrameVB, "Shared Tangent Buffer");
		ReallocateGeneric(sharedBitangentBuffer, currentVertexSize, newSize, vertexAllocatedList, vertexFreeList, sizeof(VertexBitangent_t), { .StorageBuffer = true, .VertexBuffer = true }, lastResizeFrameVB, "Shared Bitangent Buffer");
		ReallocateGeneric(sharedUV0Buffer, currentVertexSize, newSize, vertexAllocatedList, vertexFreeList, sizeof(VertexUV_t), { .StorageBuffer = true, .VertexBuffer = true }, lastResizeFrameVB, "Shared UV0 Buffer");
		ReallocateGeneric(sharedLightmapUVBuffer, currentVertexSize, newSize, vertexAllocatedList, vertexFreeList, sizeof(VertexUV_t), { .StorageBuffer = true, .VertexBuffer = true }, lastResizeFrameVB, "Shared LightmapUV Buffer");
	}
	void RenderEngine::ReallocateIndexAllocationToSize(uint32_t newSize)
	{
		ReallocateGeneric(sharedIndexBuffer, currentIndexSize, newSize, indexAllocatedList, indexFreeList, sizeof(uint32_t), {.IndexBuffer = true}, lastResizeFrameIB, "Shared Index Buffer");
	}
	void RenderEngine::ReallocateGeneric(RGLBufferPtr& reallocBuffer, uint32_t& targetBufferCurrentSize, uint32_t newSize, allocation_allocatedlist_t& allocatedList, allocation_freelist_t& freelist, uint32_t stride, RGL::BufferConfig::Type bufferType, decltype(frameCount)& lastResizeFrame, const char* debugName)
	{
		// newsize is the minimum size needed to fit the new data and nothing more
		// we want to over-allocate a bit in case more data is loaded
		newSize = closest_power_of<float>(newSize, 1.5f);


		auto oldBuffer = reallocBuffer;
		// trash old buffer
		reallocBuffer = device->CreateBuffer({
			newSize,
			bufferType,
			stride,
			RGL::BufferAccess::Private,
			{.TransferDestination = true, .Transfersource = true, .debugName = debugName}
			});

		auto extendLastRange = [&freelist,newSize,targetBufferCurrentSize]() {
			if (freelist.size() == 0) {
				// add a new range representing the space that has been made available
                freelist.push_back(Range{targetBufferCurrentSize, newSize - targetBufferCurrentSize});
				return;
			}
			// otherwise find the last range and extend its length
			auto lastIt = freelist.begin();
			for (auto it = freelist.begin(); it != freelist.end(); it++) {
				if (it->start > lastIt->start) {
					lastIt = it;
				}
			}
			lastIt->count = newSize - lastIt->start;
		};
		targetBufferCurrentSize = newSize;


		// no copying needed if the buffer began empty
		if (oldBuffer == nullptr) {
			extendLastRange();
			return;
		}

		if (lastResizeFrame != frameCount) {	// if they are equal, then this means a resize occurred on this frame. Therefore, we don't want to schedule deletion of the buffer
			gcBuffers.enqueue(oldBuffer);
		}
		


		// begin compaction

		auto commandbuffer = mainCommandQueue->CreateCommandBuffer();
		auto fence = device->CreateFence({});
		commandbuffer->Begin();

		// fill holes and copy data
		{
			uint32_t offset = 0;
			for (auto& ptr : allocatedList) {
				auto oldstart = ptr.start;
				// copy buffers from:oldBuffer, offset:oldstart, to:sharedVertexBuffer offset:offset, size: ptr->count
				commandbuffer->CopyBufferToBuffer(
					{
						.buffer = oldBuffer,
						.offset = oldstart * stride,
					},
					{
						.buffer = reallocBuffer,
						.offset = offset * stride,
					},
					ptr.count * stride
				);
				ptr.start = offset;
				offset += ptr.count;
			}
		}
		// submit and wait
		commandbuffer->End();
		commandbuffer->Commit({ fence });
		extendLastRange();
		fence->Wait();
		lastResizeFrame = frameCount;
	}
}
#endif
