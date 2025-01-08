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
        if (config.options.debugName) {
            debugName = config.options.debugName;
        }

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
        isWritable = config.options.Writable;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);    // default to PRIVATE
        nativeState = D3D12_RESOURCE_STATE_COMMON;
        // if writable, must be constructed with a UAV, otherwise use a standard SRV
        auto resourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer(size_bytes, isWritable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE);

        // decide the heap type
        
        if (config.access == RGL::BufferAccess::Shared) {

            heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            canBeTransitioned = false;
            nativeState = D3D12_RESOURCE_STATE_GENERIC_READ;   // UPLOAD requires this state, and resources cannot leave this state
            
        }

        if (config.type.StorageBuffer && canBeTransitioned) {
            nativeState = D3D12_RESOURCE_STATE_GENERIC_READ;
        }

        if (config.type.VertexBuffer && canBeTransitioned) {
            nativeState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }

        if (config.type.IndirectBuffer && canBeTransitioned) {
            nativeState = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        }

        if (config.type.IndexBuffer && canBeTransitioned) {
            nativeState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }

        if (config.options.PixelShaderResource && canBeTransitioned) {
            nativeState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        }

        if (config.options.ReadbackTarget) {
            // readback requires D3D12_RESOURCE_STATE_COPY_DEST and cannot be transitioned away from this state
            heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
            nativeState = D3D12_RESOURCE_STATE_COPY_DEST;
        }

        DX_CHECK(device->device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescriptor,
            D3D12_RESOURCE_STATE_COMMON,        // weird D3D12 garbage. We have to create in Common, but we'll pretend it isn't Common because transitioning TO common is very slow
            nullptr,
            IID_PPV_ARGS(&buffer)));

        if (config.options.debugName) {
            std::wstring wide;
            wide.resize(config.options.debugName == nullptr ? 0 : strlen(config.options.debugName));
            MultiByteToWideChar(CP_UTF8, 0, config.options.debugName, -1, wide.data(), wide.size());
            buffer->SetName(wide.c_str());
        }

        vertexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
        indexBufferView.BufferLocation = vertexBufferView.BufferLocation;   //NOTE: if this is made a union, check this

        // add it to the SRV heap 
        srvIdx = owningDevice->CBV_SRV_UAVHeap->AllocateSingle();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
            .Format = DXGI_FORMAT_R32_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = {
                .FirstElement = 0,
                .NumElements = size_bytes / sizeof(uint32_t),
                .StructureByteStride = 0,
                .Flags = D3D12_BUFFER_SRV_FLAG_RAW
            },

        };
        owningDevice->device->CreateShaderResourceView(buffer.Get(), &srvDesc, owningDevice->CBV_SRV_UAVHeap->GetCpuHandle(srvIdx));

        if (config.options.Writable) {
            uavIdx = owningDevice->CBV_SRV_UAVHeap->AllocateSingle();
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
                .Format = DXGI_FORMAT_R32_TYPELESS,
                .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                .Buffer = {
                   .FirstElement = 0,
                   .NumElements = size_bytes / sizeof(uint32_t),
                   .StructureByteStride = 0,
                   .CounterOffsetInBytes = 0,
                   .Flags = D3D12_BUFFER_UAV_FLAG_RAW
                }
            };
            owningDevice->device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uavDesc, owningDevice->CBV_SRV_UAVHeap->GetCpuHandle(uavIdx));
        }
		
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

            auto state = nativeState;
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
        // release descriptors
        owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(srvIdx);
        if (uavIdx != std::numeric_limits<decltype(uavIdx)>::max()) {
            owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(uavIdx);
        }
    }

    uint32_t BufferD3D12::GetReadonlyBindlessGPUHandle() const
    {
        
        return srvIdx;
    }

    uint32_t BufferD3D12::GetReadwriteBindlessGPUHandle() const
    {
       
        return uavIdx;
    }
}
#endif


