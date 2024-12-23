#pragma once
#include "VRAMVector.hpp"
#include <RGL/Types.hpp>
#include "Function.hpp"
#include "VRAMSparseSet.hpp"
#include "Types.hpp"

namespace RavEngine{

class BufferedVRAMStructureBase{
    
protected:
    RGLBufferPtr privateBuffer;
    std::string debugName;
    std::vector<unsigned char> syncTrackingBuffer;

    void EncodeSync(RGLDevicePtr device, RGLBufferPtr hostBuffer, RGLCommandBufferPtr commandBuffer, uint32_t elemSize, const Function<void(RGLBufferPtr)>& gcBuffersFn, bool& previousCommandResetCB);
    
    void InitializePrivateBuffer(RGLDevicePtr device, uint32_t size);

public:
    auto GetPrivateBuffer() const{
        return privateBuffer;
    }
    BufferedVRAMStructureBase() {}
    BufferedVRAMStructureBase(const std::string_view debugName) : debugName(debugName) {}

    ~BufferedVRAMStructureBase();

    MOVE_NO_COPY(BufferedVRAMStructureBase);
};

template<typename T>
class BufferedVRAMVector : public BufferedVRAMStructureBase{
    VRAMVector<T> hostBuffer;
    
public:
    BufferedVRAMVector() {}
    BufferedVRAMVector(const std::string_view debugName) : BufferedVRAMStructureBase(debugName){}
    BufferedVRAMVector(uint32_t initialSize){
        Resize(initialSize);
    }

    MOVE_NO_COPY(BufferedVRAMVector);

    virtual ~BufferedVRAMVector(){ }
    
    auto& GetHostBuffer() const{
        return hostBuffer;
    }
    
    /**
     Encodes commands to sync the host buffer with the private buffer.
     @note This function may change the value returned by GetPrivateBuffer(). Do not call GetPrivateBuffer() until after calling EncodeSync.
     @return true if commands were encoded
     */
    void EncodeSync(RGLDevicePtr device, RGLCommandBufferPtr commandBuffer, const Function<void(RGLBufferPtr)>& gcBuffersFn, bool& previousCommandResetCB){
        BufferedVRAMStructureBase::EncodeSync(device, hostBuffer.buffer, commandBuffer, sizeof(T), gcBuffersFn,  previousCommandResetCB);
    }
    
    void Resize(uint32_t newSize){
        if (hostBuffer.size() != newSize){
            hostBuffer.resize(newSize);
            syncTrackingBuffer.resize(newSize, true);   // ensure the initial copy is included
        }
    }
    
    void push_back(const T& value){
        hostBuffer.push_back(value);
        syncTrackingBuffer.resize(hostBuffer.size(), true);
    }
    
    void erase(const auto& it){
        hostBuffer.erase(it);
        auto index = it - hostBuffer.begin();
        // elements have been shifted down, so mark all elements afterward as modified
        for(auto it = syncTrackingBuffer.begin() + index; it != syncTrackingBuffer.end(); ++it){
            *it = true;
        }
    }
    
    auto begin(){
        return hostBuffer.begin();
    }
    
    void reserve(uint32_t size){
        hostBuffer.reserve(size);
        syncTrackingBuffer.reserve(size);
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
    
    auto& GetValueAtForWriting(uint32_t i){
        syncTrackingBuffer[i] = true;   // signal that this was modified
        return hostBuffer[i];
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
    
    using index_type = decltype(sparseSet)::index_type;
    using value_type = T;
    
    BufferedVRAMSparseSet() {}
    BufferedVRAMSparseSet(const std::string_view debugName) : BufferedVRAMStructureBase(debugName) {}
    virtual ~BufferedVRAMSparseSet() {}

    MOVE_NO_COPY(BufferedVRAMSparseSet);

    auto DenseSize() const{
        return sparseSet.DenseSize();
    }
    
    template<typename ... A>
    void Emplace(index_t sparse_index, A&& ... args) {
        sparseSet.Emplace(sparse_index, std::forward<A>(args) ...);
        ResizeIfNeeded();
        syncTrackingBuffer[sparseSet.SparseToDense(sparse_index)] = true;   // mark modified
    }
    
    void EraseAtSparseIndex(index_t sparseIndex){
        syncTrackingBuffer[sparseSet.SparseToDense(sparseIndex)] = true;   // mark modified
        sparseSet.EraseAtSparseIndex(sparseIndex);
    }
    
    const auto& GetReverseMap() const{
        return sparseSet.reverse_map;
    }
    
    bool HasForSparseIndex(index_type sparseIndex) const{
        return sparseSet.HasForSparseIndex(sparseIndex);
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
    void EncodeSync(RGLDevicePtr device, RGLCommandBufferPtr commandBuffer, const Function<void(RGLBufferPtr)>& gcBuffersFn, bool& previousCommandResetCB){
        if (privateBuffer == nullptr){
            InitializePrivateBuffer(device, 8 * sizeof(T));
        }
        BufferedVRAMStructureBase::EncodeSync(device, sparseSet.GetDense().get_underlying().buffer, commandBuffer, sizeof(T), gcBuffersFn, previousCommandResetCB);
    }
};
}
