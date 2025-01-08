#pragma once
#include <RGL/Types.hpp>
#include <RGL/Buffer.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <memory>

namespace RGL{
    struct DeviceMTL;
    struct BufferMTL : public IBuffer{
        OBJC_ID(MTLBuffer) buffer = nullptr;
        const std::shared_ptr<DeviceMTL> owningDevice;
        MutableSpan data;
        uint32_t stride = 0;
        uint32_t mode;
        uint32_t globalIndex = 0;
        BufferMTL(decltype(owningDevice), const BufferConfig&);
        
        //IBuffer
        void MapMemory() ;
        void UnmapMemory() ;
        void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) ;
        void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) ;
        decltype(BufferConfig::nElements) getBufferSize() const final;
        void* GetMappedDataPtr() final;
        
        void SignalRangeChanged(const Range&) final;
        virtual ~BufferMTL();
        
        uint32_t GetReadonlyBindlessGPUHandle() const final;
        uint32_t GetReadwriteBindlessGPUHandle() const final;
    };
}
