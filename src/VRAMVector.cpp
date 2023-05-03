#include "VRAMVector.hpp"
#include "GetApp.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"

namespace RavEngine {
	void VRAMVectorBase::TrashOldVector(RGLBufferPtr buffer)
	{
		buffer->UnmapMemory();
		GetApp()->GetRenderEngine().gcBuffers.enqueue(buffer);
		buffer = nullptr;
	}
}