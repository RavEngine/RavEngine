#if RGL_DX12_AVAILABLE
#include "D3D12Buffer.hpp"
#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"

namespace RGL {

	BufferD3D12::BufferD3D12(decltype(owningDevice) device, const BufferConfig& config) : owningDevice(device), myType(config.type)
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

        auto v = isWritable ? CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) : CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        // writable is marked as a UAV
        auto t = CD3DX12_RESOURCE_DESC::Buffer(size_bytes, isWritable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE );
        auto state = D3D12_RESOURCE_STATE_GENERIC_READ;

        if (config.options.TransferDestination) {
            state = D3D12_RESOURCE_STATE_COPY_DEST;
            v = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        }
        DX_CHECK(device->device->CreateCommittedResource(
            &v,
            D3D12_HEAP_FLAG_NONE,
            &t,
            state,
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
        MapMemory();
        UpdateBufferData(data, offset);
        UnmapMemory();
	}
    decltype(BufferConfig::nElements) BufferD3D12::getBufferSize() const
    {
        return mappedMemory.size;
    }
    void* BufferD3D12::GetMappedDataPtr()
    {
        return mappedMemory.data;
    }
}
#endif


