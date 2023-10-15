#if RGL_WEBGPU_AVAILABLE
#include "WGBuffer.hpp"
#include "RGLCommon.hpp"
#include "WGDevice.hpp"

namespace RGL{
    BufferWG::BufferWG(decltype(owningDevice) owningDevice, const BufferConfig& config) : owningDevice(owningDevice){
        WGPUBufferDescriptor bufferDesc{
            .nextInChain = nullptr,
            .label = config.options.debugName,
            .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead,   //TODO: use values from config
            .size = config.nElements * config.stride,
            .mappedAtCreation = false
        };

        buffer = wgpuDeviceCreateBuffer(owningDevice->device,&bufferDesc);
        data.size = config.nElements * config.stride;
        stride = config.stride;
    }

    void BufferWG::MapMemory(){
        FatalError("BufferWG::MapMemory unimplemented");
    }
    void BufferWG::UnmapMemory(){
        FatalError("BufferWG::UnmapMemory unimplemented");
    }
    void BufferWG::UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset){  
        //we currently just grab the global queue
        auto queue = wgpuDeviceGetQueue(owningDevice->device);  
        wgpuQueueWriteBuffer(queue, buffer, offset, newData.data(), newData.size());
    }
    void BufferWG::SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset){
        UpdateBufferData(data,offset);
    }
    decltype(BufferConfig::nElements) BufferWG::getBufferSize() const{
        return data.size;
    }
    void* BufferWG::GetMappedDataPtr(){
        return data.data;
    }
    
    void BufferWG::SignalRangeChanged(const Range&){
        // does nothing
    }
    BufferWG::~BufferWG(){
        wgpuBufferDestroy(buffer);
        wgpuBufferRelease(buffer);
    }
}

#endif