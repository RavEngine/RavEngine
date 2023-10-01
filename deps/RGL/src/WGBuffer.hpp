#pragma once
#include <RGL/Types.hpp>
#include <RGL/Buffer.hpp>
#include <memory>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct DeviceWG;
    struct BufferWG : public IBuffer{
        WGPUBuffer buffer = nullptr;
        const std::shared_ptr<DeviceWG> owningDevice;
        MutableSpan data;
        uint32_t stride = 0;
        uint32_t mode;
        BufferWG(decltype(owningDevice), const BufferConfig&);
        
        //IBuffer
        void MapMemory();
        void UnmapMemory();
        void UpdateBufferData(untyped_span newData, decltype(BufferConfig::nElements) offset = 0) ;
        void SetBufferData(untyped_span data, decltype(BufferConfig::nElements) offset = 0) ;
        decltype(BufferConfig::nElements) getBufferSize() const final;
        void* GetMappedDataPtr() final;
        
        void SignalRangeChanged(const Range&) final;
        virtual ~BufferWG();
    };
}
