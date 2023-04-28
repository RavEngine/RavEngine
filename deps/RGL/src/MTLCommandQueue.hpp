#pragma once
#include <RGL/Types.hpp>
#include <RGL/CommandQueue.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <memory>

namespace RGL{
struct DeviceMTL;

struct CommandQueueMTL : public ICommandQueue, public std::enable_shared_from_this<CommandQueueMTL>{
    const std::shared_ptr<DeviceMTL> owningDevice;
    OBJC_ID(MTLCommandQueue) commandQueue = nullptr;
    CommandQueueMTL(decltype(owningDevice));
    
    RGLCommandBufferPtr CreateCommandBuffer() final;
    
    QueueData GetQueueData() final;

    void WaitUntilCompleted() final;
    virtual ~CommandQueueMTL(){}
};
}
