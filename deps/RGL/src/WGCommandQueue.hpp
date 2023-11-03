#pragma once
#include <RGL/CommandQueue.hpp>
#include <emscripten/html5_webgpu.h>
#include <memory>

namespace RGL{
    struct DeviceWG;

    struct CommandQueueWG : public ICommandQueue, public std::enable_shared_from_this<CommandQueueWG>{
        WGPUQueue queue = nullptr;
        const std::shared_ptr<DeviceWG> owningDevice;
        CommandQueueWG(decltype(owningDevice));

        RGLCommandBufferPtr CreateCommandBuffer() final;
    
        QueueData GetQueueData() final;

        void WaitUntilCompleted() final{}
        virtual ~CommandQueueWG();
    };
}