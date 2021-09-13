#pragma once
#include <bgfx/bgfx.h>
#include <cstring>

namespace RavEngine {
	class TransientComputeBufferReadOnly {
	protected:
		bgfx::DynamicVertexBufferHandle handle = BGFX_INVALID_HANDLE;
		size_t index = 0;

		TransientComputeBufferReadOnly(size_t size, int flags) {
			bgfx::VertexLayout vl;
			vl.begin().add(bgfx::Attrib::Position, 1, bgfx::AttribType::Float).end();

			handle = bgfx::createDynamicVertexBuffer(size, vl, flags | BGFX_BUFFER_ALLOW_RESIZE);
		}


	public:

		TransientComputeBufferReadOnly() {}

		TransientComputeBufferReadOnly(size_t size) : TransientComputeBufferReadOnly(size, BGFX_BUFFER_COMPUTE_READ_WRITE) {}
		
		/**
		* Reset the buffer. This does not clear data.
		*/
        constexpr inline void Reset() {
			index = 0;
		}

		/**
		* Reserve empty space in the buffer.
		* @param count number of entries in the buffer
		* @param layout object describing each entry in the buffer
		* @return the index representing the beginning of the data added to the buffer
		*/
        inline decltype(index) AddEmptySpace(size_t count, const bgfx::VertexLayout& layout) {
			auto startpos = index;
			index += count * layout.getStride();
			return startpos;
		}

        constexpr inline const decltype(handle)& GetHandle() const {
			return handle;
		}

        inline void DestroyBuffer() {
			if (bgfx::isValid(handle)) {
				bgfx::destroy(handle);
				handle = BGFX_INVALID_HANDLE;
			}
		}
	};

	struct TransientComputeBuffer : public TransientComputeBufferReadOnly {
		TransientComputeBuffer() {}
		/**
		* Construct a compute buffer
		* @param size the size of the buffer, in multiples of sizeof(float)
		*/
		TransientComputeBuffer(size_t size) : TransientComputeBufferReadOnly(size, BGFX_BUFFER_COMPUTE_READ) {}

		/**
		* Add data to the buffer.
		* @param data pointer to the buffer
		* @param count number of entries in the buffer
		* @param layout object describing each entry in the buffer
		* @return the index representing the beginning of the data added to the buffer
		*/
        inline decltype(index) AddData(const uint8_t* data, size_t count, const bgfx::VertexLayout& layout) {
			auto startpos = index;
			bgfx::update(handle, index, bgfx::copy(data, count * layout.getStride()));
			index += (count * layout.getStride());
			return startpos;
		}

	};

}
