#if !RVE_SERVER
#include "BufferPool.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"
#include "Debug.hpp"

namespace RavEngine {
	void SharedBufferPool::Reset()
	{
		if (auto app = GetApp()) {
			auto& gcBuffers = app->GetRenderEngine().gcBuffers;
			// any free buffers that were not used are probably the wrong size and so we should get rid of them
			for (const auto& buffer : freePool) {
				gcBuffers.enqueue(buffer);
			}
			freePool.clear();
			for (const auto& buffer : usedPool) {
				freePool.insert(buffer);
			}
			usedPool.clear();
		}
		else {
			Debug::Assert(false, "Trying to reset bufferpool after application exit");
		}
	}
	RGLBufferPtr SharedBufferPool::GetBuffer(uint32_t sizeBytes)
	{
		RGLBufferPtr buffer;
		for (auto it = freePool.begin(); it != freePool.end(); ++it) {
			auto& freeBuffer = *it;
			if (freeBuffer->getBufferSize() == sizeBytes) {
				buffer = freeBuffer;
				freePool.erase(it);				// move it to the used pool
				
				break;
			}
		}
		if (!buffer) {
			// then we didn't find one, so create one
			if (auto app = GetApp()) {
				auto device = GetApp()->GetDevice();
				buffer = device->CreateBuffer({
					sizeBytes,
					{.StorageBuffer = 1},
					sizeof(std::byte),
					RGL::BufferAccess::Shared,
					{.Transfersource = true, .Writable = false, .debugName = "SharedBufferPool buffer"}
					});
			}
			else {
				// huh?? you're trying to create a buffer after the application has quit?
				Debug::Assert(false, "Trying to create a buffer after application exit");
			}
		}
		usedPool.insert(buffer);
		return buffer;
	}

	SharedBufferPool::~SharedBufferPool()
	{
		if (auto app = GetApp()) {
			// schedule for deletion
			auto& gcBuffers = app->GetRenderEngine().gcBuffers;
			for (const auto& buffer : usedPool) {
				gcBuffers.enqueue(buffer);
			}
			for (const auto& buffer : freePool) {
				gcBuffers.enqueue(buffer);
			}
		}
	}
}

#endif