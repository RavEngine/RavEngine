#pragma once
#include "AudioTypes.hpp"

namespace RavEngine {

    /**
     A render buffer for audio processing. Allocated and managed internally.
     */

    template<bool allocateScratchBuffer = true>
    struct SingleAudioRenderBuffer_t {
        float* data_impl = nullptr;
        float* scratch_impl = nullptr;
        const uint32_t totalBufferSize;
        uint8_t nchannels = 0;
        SingleAudioRenderBuffer_t(uint32_t nsamples, uint8_t nchannels) : nchannels(nchannels), totalBufferSize(nsamples* nchannels) {
            data_impl = new float[totalBufferSize] {0};
            if constexpr (allocateScratchBuffer) {
                scratch_impl = new float[totalBufferSize] {0};
            }
        }
        ~SingleAudioRenderBuffer_t() {
            if (data_impl) {
                delete[] data_impl;
            }
            if (scratch_impl) {
                delete[] scratch_impl;
            }
        }
        SingleAudioRenderBuffer_t(SingleAudioRenderBuffer_t&& other) : nchannels(other.nchannels), data_impl(other.data_impl), scratch_impl(other.scratch_impl) {
            other.data_impl = nullptr;
            other.scratch_impl = nullptr;
        }
        PlanarSampleBufferInlineView GetDataBufferView() const {
            return { data_impl, totalBufferSize, totalBufferSize / nchannels };
        }
        PlanarSampleBufferInlineView GetScratchBufferView() const {
            return { scratch_impl, totalBufferSize, totalBufferSize / nchannels };
        }
    };

    using SingleAudioRenderBuffer = SingleAudioRenderBuffer_t<true>;
    using SingleAudioRenderBufferNoScratch = SingleAudioRenderBuffer_t<false>;
}