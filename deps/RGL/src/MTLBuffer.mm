#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLBuffer.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"

namespace RGL{

BufferMTL::BufferMTL(decltype(owningDevice) owningDevice, const BufferConfig& config) : owningDevice(owningDevice){
    
    if (config.access == RGL::BufferAccess::Shared){
        mode = MTLResourceStorageModeShared;
    }
    else{
        mode = MTLResourceStorageModePrivate;
    }
    
    MTL_CHECK(buffer = [owningDevice->device newBufferWithLength:config.nElements * config.stride options: mode]);
    
    data.size = config.nElements * config.stride;
    stride = config.stride;
}

void BufferMTL::MapMemory(){
    data.data = [buffer contents];
}

void BufferMTL::UnmapMemory(){
    data.data = nullptr;
#if !TARGET_OS_IPHONE
    if (mode == MTLResourceStorageModeManaged){
        [buffer didModifyRange:NSMakeRange(0, data.size)];
    }
#endif
}

void BufferMTL::SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset){
    if (mode == MTLResourceStorageModeShared){
        UpdateBufferData(data, offset);
        UnmapMemory();
    }
    else{
        // create a staging buffer and populate it with the data
        auto tmpbuffer = [owningDevice->device newBufferWithLength:data.size() options:MTLResourceStorageModeShared];
        auto dataptr = [tmpbuffer contents];
        std::memcpy(dataptr, data.data(), data.size());
        
        // create copy commands
        auto commandBuffer = [owningDevice->uploadQueue commandBuffer];
        auto blitEncoder = [commandBuffer blitCommandEncoder];
        [blitEncoder copyFromBuffer:tmpbuffer sourceOffset:0 toBuffer:buffer destinationOffset:offset size:data.size()];
        [blitEncoder endEncoding];
        
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
    }
}

void BufferMTL::UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset){
    if (!data.data){
        MapMemory();
    }
    Assert(newData.size() + offset <= data.size, "Data would exceed end of buffer!");
    std::memcpy(static_cast<std::byte*>(data.data) + offset, newData.data(), newData.size());
    SignalRangeChanged({
        .offset = offset,
        .length = static_cast<uint32_t>(newData.size())
    });
}

decltype(BufferConfig::nElements) BufferMTL::getBufferSize() const{
    return data.size;
}

void* BufferMTL::GetMappedDataPtr() {
    return data.data;
}

void BufferMTL::SignalRangeChanged(const Range & range){
#if TARGET_OS_IPHONE
#else
    if (mode == MTLResourceStorageModeManaged){
        [buffer didModifyRange:NSMakeRange(range.offset, range.length)];
    }
#endif
}
}
#endif
