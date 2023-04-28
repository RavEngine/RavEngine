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
        BufferMTL(decltype(owningDevice), const BufferConfig&);
        
        //IBuffer
        void MapMemory() ;
        void UnmapMemory() ;
        void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) ;
        void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) ;
        decltype(BufferConfig::nElements) getBufferSize() const final;
        void* GetMappedDataPtr() final;
        virtual ~BufferMTL(){}
    };
}
