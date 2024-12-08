#pragma once
#include "VRAMVector.hpp"
#include <RGL/Types.hpp>
#include "Function.hpp"
#include "VRAMSparseSet.hpp"

namespace RavEngine{

class BufferedVRAMStructureBase{
    
protected:
    RGLBufferPtr privateBuffer;
    std::vector<unsigned char> syncTrackingBuffer;

    bool EncodeSync(RGLDevicePtr device, RGLBufferPtr hostBuffer, RGLCommandBufferPtr commandBuffer, uint32_t elemSize, const Function<void(RGLBufferPtr)>& gcBuffersFn);
    
    void InitializePrivateBuffer(RGLDevicePtr device, uint32_t size);

public:
    auto GetPrivateBuffer() const{
        return privateBuffer;
    }
};

template<typename T>
class BufferedVRAMVector : public BufferedVRAMStructureBase{
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
        return BufferedVRAMStructureBase::EncodeSync(device, GetHostBuffer().buffer, commandBuffer, sizeof(T), gcBuffersFn);
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

/**
 The private buffer mirrors the Dense set.
 */
template<typename index_t, typename T>
class BufferedVRAMSparseSet : public BufferedVRAMStructureBase{
    VRAMSparseSet<index_t, T> sparseSet;
    
    void ResizeIfNeeded(){
        if (sparseSet.DenseSize() > syncTrackingBuffer.size()){
            syncTrackingBuffer.resize(sparseSet.DenseSize(), true);   // ensure the initial copy is included
        }
    }
    
public:
    
    auto DenseSize() const{
        return sparseSet.DenseSize();
    }
    
    template<typename ... A>
    void Emplace(index_t sparse_index, A&& ... args) {
        sparseSet.Emplace(sparse_index, std::forward<A>(args) ...);
    }
    
    void EraseAtSparseIndex(index_t sparseIndex){
        sparseSet.EraseAtSparseIndex(sparseIndex);
    }
    
//    const auto& GetHostDense() const{
//        return sparseSet.GetDense();
//    }
    
    const auto& GetAtDenseIndex(index_t dense_index){
        return sparseSet.GetDense()[dense_index];
    }
    
    auto& GetHostDenseForWriting(index_t dense_index){
        ResizeIfNeeded();
        syncTrackingBuffer[dense_index] = true;
        return sparseSet.GetDense()[dense_index];
    }
    
    auto& GetSparseIndexForDense(index_t index) {
        return sparseSet.GetSparseIndexForDense(index);
    }
    
    auto& GetForSparseIndexForWriting(index_t sparse_index){
        ResizeIfNeeded();
        syncTrackingBuffer[sparseSet.SparseToDense(sparse_index)] = true;
        return sparseSet.GetForSparseIndex(sparse_index);
    }
    
    
    /**
     Encodes commands to sync the host buffer with the private buffer.
     @note This function may change the value returned by GetPrivateBuffer(). Do not call GetPrivateBuffer() until after calling EncodeSync.
     @return true if commands were encoded
     */
    bool EncodeSync(RGLDevicePtr device, RGLCommandBufferPtr commandBuffer, const Function<void(RGLBufferPtr)>& gcBuffersFn){
        if (privateBuffer == nullptr){
            InitializePrivateBuffer(device, 8 * sizeof(T));
        }
        return BufferedVRAMStructureBase::EncodeSync(device, sparseSet.GetDense().get_underlying().buffer, commandBuffer, sizeof(T), gcBuffersFn);
    }
};
}
