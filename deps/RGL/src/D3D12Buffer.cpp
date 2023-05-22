#if RGL_DX12_AVAILABLE
#include "D3D12Buffer.hpp"
#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include <D3D12MemAlloc.h>
#include <ResourceUploadBatch.h>

using namespace Microsoft::WRL;

namespace RGL {

    D3D12_RESOURCE_STATES typeToState(RGL::BufferConfig::Type type) {
        if (type.StorageBuffer) {
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else if (type.IndexBuffer) {
            return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }
        else if (type.VertexBuffer) {
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }
        else if (type.IndirectBuffer) {
            return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        }
        else {
            return D3D12_RESOURCE_STATE_COPY_DEST;
        }
    }

	BufferD3D12::BufferD3D12(decltype(owningDevice) device, const BufferConfig& config) : owningDevice(device), myType(config.type), accessType(config.access)
	{
        const auto size_bytes = config.nElements * config.stride;
        mappedMemory.size = size_bytes;
        if (config.type.IndexBuffer) {
            indexBufferView.SizeInBytes = size_bytes;
            indexBufferView.Format = config.stride == sizeof(uint16_t) ? decltype(indexBufferView.Format)::DXGI_FORMAT_R16_UINT : decltype(indexBufferView.Format)::DXGI_FORMAT_R32_UINT;
        }
        else {
            vertexBufferView.SizeInBytes = size_bytes;
            vertexBufferView.StrideInBytes = config.stride;
        }
        const bool isWritable = config.options.Writable;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);    // default to PRIVATE
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        // if writable, must be constructed with a UAV, otherwise use a standard SRV
        auto resourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer(size_bytes, isWritable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE);

        // decide the heap type
        if (config.options.ReadbackTarget) {
            // readback requires D3D12_RESOURCE_STATE_COPY_DEST and cannot be transitioned away from this state
            heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
            initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        }
        else if (config.access == RGL::BufferAccess::Shared) {
            heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            initialState = D3D12_RESOURCE_STATE_GENERIC_READ;   // UPLOAD requires this state, and resources cannot leave this state
        }

        DX_CHECK(device->device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescriptor,
            initialState,
            nullptr,
            IID_PPV_ARGS(&buffer)));

        vertexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
        indexBufferView.BufferLocation = vertexBufferView.BufferLocation;   //NOTE: if this is made a union, check this
		
	}
	void BufferD3D12::MapMemory()
	{
        D3D12_RANGE range{
            .Begin = 0,
            .End = mappedMemory.size
        };
        buffer->Map(0, &range, &mappedMemory.data);
	}

	void BufferD3D12::UnmapMemory()
	{
        D3D12_RANGE range{
           .Begin = 0,
           .End = mappedMemory.size
        };
        buffer->Unmap(0, &range);
        mappedMemory.data = nullptr;
        mappedMemory.size = 0;
	}
	void BufferD3D12::UpdateBufferData(untyped_span data, decltype(BufferConfig::nElements) offset)
	{
        if (!mappedMemory.data) {
            MapMemory();
        }
        Assert(data.size() + offset <= mappedMemory.size, "Attempting to write more data than the buffer can hold");
        memcpy(static_cast<std::byte*>(mappedMemory.data) + offset, data.data(), data.size());
	}
	void BufferD3D12::SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset)
	{
        if (accessType == decltype(accessType)::Shared) {
            UpdateBufferData(data, offset);
            UnmapMemory();
        }
        else {

            // create the staging buffer
            D3D12MA::ALLOCATION_DESC textureUploadAllocDesc{
                .HeapType = D3D12_HEAP_TYPE_UPLOAD
            };
            D3D12_RESOURCE_DESC bufferUploadResourceDesc{
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = data.size(),
                .Height = 1,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0,
                },
                .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = D3D12_RESOURCE_FLAG_NONE
            };
            ComPtr<ID3D12Resource> bufferUpload;
            D3D12MA::Allocation* bufferUploadAllocation;
            DX_CHECK(owningDevice->allocator->CreateResource(
                &textureUploadAllocDesc,
                &bufferUploadResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr, // pOptimizedClearValue
                &bufferUploadAllocation,
                IID_PPV_ARGS(&bufferUpload)));
            bufferUpload->SetName(L"bufferUpload");

            // put the data in the staging buffer
            void* writePtr;
            D3D12_RANGE range{
                .Begin = 0,
                .End = data.size()
            };
            bufferUpload->Map(0, &range, &writePtr);
            std::memcpy(writePtr, data.data(), data.size());
            bufferUpload->Unmap(0, &range);

            // upload the data to the GPU
            auto commandList = owningDevice->internalQueue->CreateCommandList();

            auto state = D3D12_RESOURCE_STATE_GENERIC_READ;
            auto beginTransition = CD3DX12_RESOURCE_BARRIER::Transition(
                buffer.Get(),
                state,
                D3D12_RESOURCE_STATE_COPY_DEST
            );
            commandList->ResourceBarrier(1, &beginTransition);
            commandList->CopyBufferRegion(buffer.Get(), offset, bufferUpload.Get(), 0, data.size());

            auto endTransition = CD3DX12_RESOURCE_BARRIER::Transition(
                buffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                state
            );
            commandList->ResourceBarrier(1, &endTransition);

            commandList->Close();
            auto fenceValue = owningDevice->internalQueue->ExecuteCommandList(commandList);
            owningDevice->internalQueue->WaitForFenceValue(fenceValue);

            bufferUploadAllocation->Release();
        }
       
	}
    decltype(BufferConfig::nElements) BufferD3D12::getBufferSize() const
    {
        return mappedMemory.size;
    }
    void* BufferD3D12::GetMappedDataPtr()
    {
        return mappedMemory.data;
    }

    void BufferD3D12::SignalRangeChanged(const Range & range){
        
    }
    BufferD3D12::~BufferD3D12()
    {
        if (mappedMemory.data != nullptr) {
            UnmapMemory();
        }
    }
}
#endif


