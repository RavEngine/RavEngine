#pragma once
#include "AudioTypes.hpp"

namespace RavEngine {

    /**
     A render buffer for audio processing. Allocated and managed internally.
     */

    struct SingleAudioRenderBuffer_t_Base {
    protected:
        static uint16_t impl_GetBufferSize();
    };

    template<bool allocateScratchBuffer = true>
    struct SingleAudioRenderBuffer_t : protected SingleAudioRenderBuffer_t_Base {
        float* data_impl = nullptr;
        float* scratch_impl = nullptr;
        uint8_t nchannels = 0;
        SingleAudioRenderBuffer_t(uint32_t nsamples, uint8_t nchannels) : nchannels(nchannels) {
            data_impl = new float[nsamples * nchannels] {0};
            if constexpr (allocateScratchBuffer) {
                scratch_impl = new float[nsamples * nchannels] {0};
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
            return { data_impl, static_cast<size_t>(impl_GetBufferSize() * nchannels), static_cast<size_t>(impl_GetBufferSize()) };
        }
        PlanarSampleBufferInlineView GetScratchBufferView() const {
            return { scratch_impl, static_cast<size_t>(impl_GetBufferSize() * nchannels), static_cast<size_t>(impl_GetBufferSize()) };
        }
    };

    using SingleAudioRenderBuffer = SingleAudioRenderBuffer_t<true>;
    using SingleAudioRenderBufferNoScratch = SingleAudioRenderBuffer_t<false>;
}