#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLBuffer.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"

namespace RGL{

BufferMTL::BufferMTL(decltype(owningDevice) owningDevice, const BufferConfig& config) : owningDevice(owningDevice){
    //TODO: make access configurable and options
    mode =
#if TARGET_OS_IPHONE
    MTLResourceStorageModeShared;
#else
    MTLResourceStorageModeManaged;
#endif
    
    if (config.access == RGL::BufferAccess::Shared){
        mode = MTLResourceStorageModeShared;
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
    MapMemory();
    UpdateBufferData(data, offset);
    UnmapMemory();
}

void BufferMTL::UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset){
    Assert(data.data != nullptr, "Must call MapMemory before updating a buffer");
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
    if (mode == MTLResourceStorageModeManaged){
        [buffer didModifyRange:NSMakeRange(range.offset, range.length)];
    }
}
}
#endif
