#if RGL_WEBGPU_AVAILABLE
#include "WGCommandQueue.hpp"
#include "WGCommandBuffer.hpp"
#include "WGDevice.hpp"
#include <iostream>

namespace RGL{
    CommandQueueWG::CommandQueueWG(decltype(owningDevice) owningDevice) : owningDevice(owningDevice){
        queue = wgpuDeviceGetQueue(owningDevice->device);
    }

    RGLCommandBufferPtr CommandQueueWG::CreateCommandBuffer(){
        return std::make_shared<CommandBufferWG>(shared_from_this());
    }

    QueueData CommandQueueWG::GetQueueData(){
        return {};
    }

    CommandQueueWG::~CommandQueueWG(){
        wgpuQueueRelease(queue);
    }
}
#endif