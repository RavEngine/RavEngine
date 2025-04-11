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
    
    if (config.options.debugName != nullptr){
        [buffer setLabel:[NSString stringWithUTF8String:config.options.debugName]];
    }
    
    data.size = config.nElements * config.stride;
    stride = config.stride;
    
    // add to the bindless heap
    globalIndex = owningDevice->bufferFreelist.Allocate();
    auto addr = buffer.gpuAddress;
    static_cast<uint64_t*>(owningDevice->globalBufferBuffer.contents)[globalIndex] = addr;
    
}

BufferMTL::~BufferMTL(){
    owningDevice->bufferFreelist.Deallocate(globalIndex);
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

uint32_t BufferMTL::GetReadonlyBindlessGPUHandle() const{
    return globalIndex;
}

uint32_t BufferMTL::GetReadwriteBindlessGPUHandle() const{
    return globalIndex;
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
