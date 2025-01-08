#pragma once
#include <RGL/Types.hpp>
#include <RGL/Buffer.hpp>
#include <directx/d3d12.h>
#include "RGLD3D12.hpp"
#include "D3D12TrackedResource.hpp"
#include <directx/d3dx12.h>
#include <memory>

namespace RGL {
	struct DeviceD3D12;

	struct BufferD3D12 : public IBuffer, public D3D12TrackedResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};	// TODO: Union or something to optimize this
		BufferConfig::Type myType;
		const RGL::BufferAccess accessType;
		bool isWritable = false;
		bool canBeTransitioned = true;
		uint32_t srvIdx = 0, uavIdx = std::numeric_limits<uint32_t>::max();
		std::string debugName;

		const std::shared_ptr<DeviceD3D12> owningDevice;
		MutableSpan mappedMemory;

		BufferD3D12(decltype(owningDevice), const BufferConfig&);

		/**
		* Map system RAM for updating this buffer.
		*/
		void MapMemory() final;

		/**
		* Unmap system RAM for updating this buffer.
		*/
		void UnmapMemory() final;

		/**
		Update the contents of this buffer. If memory is not mapped, it will become mapped. The memory remains mapped. Intended to be used with UniformBuffers or other data that changes frequently.
		@param newData the data to write into the buffer.
		*/
		void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) final;

		/**
		Set the contents of this buffer. Intended to be used with VertexBuffers or other data that changes infrequently or never.
		@param newData the data to write into the buffer.
		*/
		void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) final;

		decltype(BufferConfig::nElements) getBufferSize() const;

		void* GetMappedDataPtr() final;
        
        void SignalRangeChanged(const Range&) final;

		virtual ~BufferD3D12();

		uint32_t GetReadonlyBindlessGPUHandle() const;
		uint32_t GetReadwriteBindlessGPUHandle() const;

		ID3D12Resource* GetResource() const final {
			return buffer.Get();
		}
	};
}
