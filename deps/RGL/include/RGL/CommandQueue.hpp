#pragma once
#include <RGL/Types.hpp>
#include <memory>

struct ID3D12CommandQueue;

namespace RGL {

	enum class QueueType {
		AllCommands, CopyOnly, ComputeOnly
	};

	struct ICommandBuffer;

	union QueueData {
		struct {
			ID3D12CommandQueue* commandQueue;
		} d3d12Data;
		struct {
			
		} vkData;
        struct {
            
        } mtlData;
	};

	struct ICommandQueue {
		virtual RGLCommandBufferPtr CreateCommandBuffer() = 0;
		virtual void WaitUntilCompleted() = 0;
		virtual QueueData GetQueueData() = 0;
	};
}
