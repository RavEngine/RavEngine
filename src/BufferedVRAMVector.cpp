#if !RVE_SERVER
#include "BufferedVRAMVector.hpp"
#include <RGL/CommandBuffer.hpp>
#include "Debug.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"

namespace RavEngine{
void BufferedVRAMStructureBase::InitializePrivateBuffer(RGLDevicePtr device, uint32_t size){
    Debug::Assert(!privateBuffer, "Cannot be invoked after creation!");
    privateBuffer = device->CreateBuffer({
        size,
        {.StorageBuffer = true},
        sizeof(std::byte),
        RGL::BufferAccess::Private,
        {.TransferDestination = true, .Transfersource = true,  .debugName = debugName.data()}
    });
}

BufferedVRAMStructureBase::~BufferedVRAMStructureBase()
{
    if (auto app = GetApp()) {
        app->GetRenderEngine().gcBuffers.enqueue(privateBuffer);
    }
}

void BufferedVRAMStructureBase::EncodeSync(RGLDevicePtr device, RGLBufferPtr hostBuffer, RGLCommandBufferPtr transformSyncCommandBuffer, uint32_t elemSize, const Function<void(RGLBufferPtr)>& gcBuffersFn, bool& needsSync){
    uint32_t newPrivateSize = 0;
    {
        const uint32_t hostSize = hostBuffer->getBufferSize();
        if (!(privateBuffer) || privateBuffer->getBufferSize() != hostSize) {
            newPrivateSize = hostSize;
        }
    }

    bool beginCalled = false;
    // sync transforms to GPU
    
    int8_t gapCounter = 0;
    uint32_t rangeBegin = 0;
    auto& syncTrackBuffer = syncTrackingBuffer;
    auto beginCB = [&] {
        if (!needsSync) {
            transformSyncCommandBuffer->Reset();
            transformSyncCommandBuffer->Begin();
        }
        if (!beginCalled) {
            transformSyncCommandBuffer->BeginRenderDebugMarker(Format("Sync {}", debugName));
        }
        beginCalled = true;
        needsSync = true;
    };

    if (newPrivateSize > 0) {
        beginCB();
        RGLBufferPtr oldBuffer;
        if (privateBuffer) {
            oldBuffer = privateBuffer;
            gcBuffersFn(oldBuffer);
        }


        privateBuffer = device->CreateBuffer({
            newPrivateSize,
            {.StorageBuffer = true},
            sizeof(std::byte),
            RGL::BufferAccess::Private,
            {.TransferDestination = true, .debugName = debugName.c_str()}
        });

        if (oldBuffer) {
            // copy the old data over
            transformSyncCommandBuffer->CopyBufferToBuffer(
                {
                    .buffer = oldBuffer,
                    .offset = 0,
                },
                {
                    .buffer = privateBuffer,
                    .offset = 0,
                },
                oldBuffer->getBufferSize());
        }
    }

    auto endRange = [&](uint32_t end) {
        // this is the end of the range. add a command
        //Debug::Log("Range: {} - {}", rangeBegin, i);
        beginCB();
        const auto bufferOffset = rangeBegin * elemSize;
        const auto copySize = (end - rangeBegin) * elemSize;
        transformSyncCommandBuffer->CopyBufferToBuffer(
            {
                .buffer = hostBuffer,
                .offset = bufferOffset
            },
            {
                .buffer = privateBuffer,
                .offset = bufferOffset
            }, copySize);

    };

    for (uint32_t i = 0; i < syncTrackBuffer.size(); i++) {
        auto& modified = syncTrackBuffer[i];

        if (modified && gapCounter == 0) {
            gapCounter = 2; // we can have gaps of up to 2 matrices before making a new range
            rangeBegin = i;
        }
        if (!modified) {
            auto prevValue = gapCounter;
            gapCounter = std::max(0, gapCounter - 1);   // decrement
            if (gapCounter == 0 && prevValue != 0) {
                endRange(i);
            }
        }
        if (modified) {
            modified = false;
        }
    }

    // if the loop ends and gapCounter != 0, then create another range
    if (gapCounter != 0) {
        endRange(uint32_t(syncTrackBuffer.size()));
    }        
    
    if (beginCalled) {
        transformSyncCommandBuffer->EndRenderDebugMarker();
    }
}
}

#endif
