#pragma once
#include <RGL/Span.hpp>

namespace RGL {


	struct BufferFlags {
		bool TransferDestination : 1 = false;
		bool Transfersource : 1 = false;
		bool ReadbackTarget : 1 = false;
		bool Writable : 1 = false;
	};

	enum class BufferAccess : uint8_t {
		Private,
		Shared
	};

	struct BufferConfig {
		uint32_t nElements = 0;
		uint32_t stride = 0;

		struct Type {
			bool UniformBuffer : 1 = false;
			bool StorageBuffer : 1 = false;
			bool IndexBuffer : 1 = false;
			bool VertexBuffer : 1 = false;
			bool IndirectBuffer : 1 = false;
		} type;

		BufferAccess access;
		BufferFlags options;

		BufferConfig(decltype(nElements) size, decltype(type) type, decltype(stride) stride, decltype(access) access, decltype(options) options = {}) : nElements(size), type(type), stride(stride), access(access), options(options) {}

		template<typename T>
		BufferConfig(decltype(type) type, decltype(stride) stride, decltype(access) access, decltype(options) options = {}) : BufferConfig(sizeof(T) / stride, type, stride, access, options) {}

		template<typename T>
		BufferConfig(decltype(type) type, decltype(stride) stride, const T& t, decltype(access) access, decltype(options) options = {}) : BufferConfig(sizeof(T) / stride, type, stride, access, options) {}
	};

    struct Range{
        uint32_t offset = 0, length = 0;
    };


	struct IBuffer {
		/**
		* Map system RAM for updating this buffer. 
		*/
		virtual void MapMemory() = 0;

		/**
		* Unmap system RAM for updating this buffer.
		*/
		virtual void UnmapMemory() = 0;
        
        virtual void SignalRangeChanged(const Range&) = 0;

		/**
		Update the contents of this buffer. If memory is not mapped, it will become mapped. The memory remains mapped. Intended to be used with UniformBuffers or other data that changes frequently.
		@param newData the data to write into the buffer.
		*/
		virtual void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) = 0;

		/**
		Set the contents of this buffer. Intended to be used with VertexBuffers or other data that changes infrequently or never.
		@param newData the data to write into the buffer.
		*/
		virtual void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) = 0;
        
        virtual decltype(BufferConfig::nElements) getBufferSize() const = 0;

		virtual void* GetMappedDataPtr() = 0;
	};
}
