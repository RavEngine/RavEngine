#pragma once
#include "Vector.hpp"
#include <RGL/Types.hpp>
#include <RGL/Buffer.hpp>

namespace RavEngine {
	/**
* Creates buffers that are Shared + TransferSource
*/
	struct SharedBufferPool {
		void Reset();

		RGLBufferPtr GetBuffer(uint32_t sizeBytes);
		~SharedBufferPool();
	private:
		UnorderedVector<RGLBufferPtr> usedPool;
		UnorderedVector<RGLBufferPtr> freePool;
	};
}