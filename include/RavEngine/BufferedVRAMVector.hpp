#pragma once
#include "VRAMVector.hpp"
#include <RGL/Types.hpp>
#include "Function.hpp"

namespace RavEngine{

class BufferedVRAMVectorBase{
    
protected:
    RGLBufferPtr privateBuffer;
    std::vector<unsigned char> syncTrackingBuffer;

    bool EncodeSync(RGLDevicePtr device, RGLBufferPtr hostBuffer, RGLCommandBufferPtr commandBuffer, uint32_t elemSize, const Function<void(RGLBufferPtr)>& gcBuffersFn);

public:
    auto GetPrivateBuffer() const{
        return privateBuffer;
    }
};

template<typename T>
class BufferedVRAMVector : public BufferedVRAMVectorBase{
    VRAMVector<T> hostBuffer;
    
public:
    
    auto& GetHostBuffer() const{
        return hostBuffer;
    }
    
    /**
     Encodes commands to sync the host buffer with the private buffer.
     @note This function may change the value returned by GetPrivateBuffer(). Do not call GetPrivateBuffer() until after calling EncodeSync.
     @return true if commands were encoded
     */
    bool EncodeSync(RGLDevicePtr device, RGLCommandBufferPtr commandBuffer, const Function<void(RGLBufferPtr)>& gcBuffersFn){
        return BufferedVRAMVectorBase::EncodeSync(device, hostBuffer.buffer, commandBuffer, sizeof(T), gcBuffersFn);
    }
    
    void Resize(uint32_t newSize){
        hostBuffer.resize(newSize);
        syncTrackingBuffer.resize(newSize, true);   // ensure the initial copy is included
    }
    
    auto Size() const{
        return hostBuffer.size();
    }
    
    const auto& operator[](uint32_t i) const{
        return hostBuffer[i];
    }
    
    void SetValueAt(uint32_t i, const T& value){
        hostBuffer[i] = value;
        syncTrackingBuffer[i] = true;    // signal that this was modified
    }
};
}
