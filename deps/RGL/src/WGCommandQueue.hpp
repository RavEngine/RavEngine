#pragma once
#include <RGL/CommandQueue.hpp>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct DeviceWG;

    struct CommandQueueWG : public ICommandQueue{
        WGPUQueue queue = nullptr;
        const std::shared_ptr<DeviceWG> owningDevice;
        CommandQueueWG(decltype(owningDevice));

        RGLCommandBufferPtr CreateCommandBuffer() final;
    
        QueueData GetQueueData() final;

        void WaitUntilCompleted() final{}
        virtual ~CommandQueueWG();
    };
}