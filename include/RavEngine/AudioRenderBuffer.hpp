#pragma once
#include "AudioTypes.hpp"
#include "SpinLock.hpp"

namespace RavEngine {

    struct SingleAudioRenderBuffer_t_readwritetrack {
        mutable uint32_t readerCount = 0;           // fake const is a code smell, but required (this class is not public API)
        mutable bool writersActive = false;
        mutable SpinLock mtx;

        void AcquireRead() const;
        void ReleaseRead() const;
        void AcquireWrite() const;
        void ReleaseWrite() const;
    };

    template<bool isReader>
    struct PlanarSampleBufferInlineViewAcessControlled : public PlanarSampleBufferInlineView {
        const SingleAudioRenderBuffer_t_readwritetrack* owner = nullptr;

        void OnConstruct() {
            if constexpr (isReader) {
                owner->AcquireRead();
            }
            else {
                owner->AcquireWrite();
            }
        }

        PlanarSampleBufferInlineViewAcessControlled(decltype(owner) owner, decltype(combined_buffers)::value_type* data, const decltype(combined_buffers)::size_type nframestotal, decltype(sizeOfOneChannelInFrames) sizeOfOneChannelInFrames) : owner(owner),
            PlanarSampleBufferInlineView(data, nframestotal, sizeOfOneChannelInFrames){
            OnConstruct();
        }

        ~PlanarSampleBufferInlineViewAcessControlled() {
            if constexpr (isReader) {
                owner->ReleaseRead();
            }
            else {
                owner->ReleaseWrite();
            }
        }
        PlanarSampleBufferInlineViewAcessControlled(const PlanarSampleBufferInlineViewAcessControlled& other) = delete; // these represent ownership now and so can't be copied
    };

    /**
     A render buffer for audio processing. Allocated and managed internally.
     */

    template<bool allocateScratchBuffer = true>
    struct SingleAudioRenderBuffer_t {
    protected:

        float* data_impl = nullptr;
        float* scratch_impl = nullptr;
        const uint32_t totalBufferSize;
        uint8_t nchannels = 0;

        SingleAudioRenderBuffer_t_readwritetrack dataTracker, scratchTracker;
    public:


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
        PlanarSampleBufferInlineViewAcessControlled<false> GetWritableDataBufferView() {
            return { &dataTracker, data_impl, totalBufferSize, GetSizeOneChannel()};
        }
        PlanarSampleBufferInlineViewAcessControlled<false> GetWritableScratchBufferView() {
            return { &scratchTracker, scratch_impl, totalBufferSize, GetSizeOneChannel() };
        }

        const PlanarSampleBufferInlineViewAcessControlled<true> GetReadonlyDataBufferView() const {
            return { &dataTracker, data_impl, totalBufferSize, GetSizeOneChannel() };
        }

        const PlanarSampleBufferInlineViewAcessControlled<true> GetReadonlyScratchBufferView() const {
            return { &scratchTracker, scratch_impl, totalBufferSize, GetSizeOneChannel() };
        }

        auto GetSizeOneChannel() const {
            return totalBufferSize / nchannels;
        }
    };

    using SingleAudioRenderBuffer = SingleAudioRenderBuffer_t<true>;
    using SingleAudioRenderBufferNoScratch = SingleAudioRenderBuffer_t<false>;
}