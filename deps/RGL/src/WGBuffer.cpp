#if RGL_WEBGPU_AVAILABLE
#include "WGBuffer.hpp"
#include "RGLCommon.hpp"
#include "WGDevice.hpp"

namespace RGL{
    BufferWG::BufferWG(decltype(owningDevice) owningDevice, const BufferConfig&) : owningDevice(owningDevice){
        WGPUBufferDescriptor bufferDesc{
            .nextInChain = nullptr

        };

        buffer = wgpuDeviceCreateBuffer(owningDevice->device,&bufferDesc);
    }

    void BufferWG::MapMemory(){

    }
    void BufferWG::UnmapMemory(){

    }
    void BufferWG::UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset){

    }
    void BufferWG::SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset){

    }
    decltype(BufferConfig::nElements) BufferWG::getBufferSize() const{

    }
    void* BufferWG::GetMappedDataPtr(){

    }
    
    void BufferWG::SignalRangeChanged(const Range&){

    }
    BufferWG::~BufferWG(){
        wgpuBufferRelease(buffer);
    }
}

#endif