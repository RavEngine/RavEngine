#if !RAVENGINE_SERVER
#include "BufferedVRAMVector.hpp"
#include <RGL/CommandBuffer.hpp>

namespace RavEngine{
bool BufferedVRAMVectorBase::EncodeSync(RGLDevicePtr device, RGLBufferPtr hostBuffer, RGLCommandBufferPtr transformSyncCommandBuffer, uint32_t elemSize, const Function<void(RGLBufferPtr)>& gcBuffersFn){
    uint32_t newPrivateSize = 0;
    {
        const uint32_t hostSize = hostBuffer->getBufferSize();
        if (!(privateBuffer) || privateBuffer->getBufferSize() != hostSize) {
            newPrivateSize = hostSize;
        }
    }


    // sync transforms to GPU
    {
        int8_t gapCounter = 0;
        uint32_t rangeBegin = 0;
        bool needsSync = false;
        auto& syncTrackBuffer = syncTrackingBuffer;

        auto beginCB = [&] {
            if (!needsSync) {
                transformSyncCommandBuffer->Reset();
                transformSyncCommandBuffer->Begin();
            }
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
                {.TransferDestination = true, .debugName = "World transform private buffer"}
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
        
        return needsSync;
    }
}
}

#endif
