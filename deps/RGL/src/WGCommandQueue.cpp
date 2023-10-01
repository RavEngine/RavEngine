#if RGL_WEBGPU_AVAILABLE
#include "WGCommandQueue.hpp"
#include "WGDevice.hpp"
#include <iostream>

namespace RGL{
    CommandQueueWG::CommandQueueWG(decltype(owningDevice) owningDevice) : owningDevice(owningDevice){
        queue = wgpuDeviceGetQueue(owningDevice->device);
    }

    RGLCommandBufferPtr CommandQueueWG::CreateCommandBuffer(){
        return nullptr;
    }

    QueueData CommandQueueWG::GetQueueData(){
        return {};
    }

    CommandQueueWG::~CommandQueueWG(){
        wgpuQueueRelease(queue);
    }
}
#endif