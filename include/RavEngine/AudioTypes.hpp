#pragma once
#include <span>
#include "Ref.hpp"
#include "DataStructures.hpp"

namespace RavEngine{
    class AudioAsset;

    /**
     Represents audio samples in interleaved float format. For example, dual-channel audio would be represented
     as LRLRLRLR....
     */
    using InterleavedSampleBufferView = std::span<float,std::dynamic_extent>;

    /**
     Represents planar audio by abstracting a single linear buffer.
     Expected that the single buffer is organized with one channel succeeding the next (with 2 channels: LLLLLLLL....LLRRRR...RR).
     Does not own the data.
     */
    class PlanarSampleBufferInlineView{
        std::span<float,std::dynamic_extent> combined_buffers;
        size_t sizeOfOneChannelInFrames = 0;
        
        auto channel_at(size_t i) const{
			auto ptr = combined_buffers.data()+(i*sizeOfOneChannelInFrames);
			decltype(combined_buffers) retview{ptr,sizeOfOneChannelInFrames};
			return retview;
        }
        
    public:
        using value_type = decltype(combined_buffers)::value_type;
        PlanarSampleBufferInlineView(const decltype(combined_buffers)& buf, decltype(sizeOfOneChannelInFrames) sizeOfOneChannelInFrames) : combined_buffers(buf), sizeOfOneChannelInFrames(sizeOfOneChannelInFrames){}
        
        PlanarSampleBufferInlineView(decltype(combined_buffers)::value_type* data, const decltype(combined_buffers)::size_type nframestotal, decltype(sizeOfOneChannelInFrames) sizeOfOneChannelInFrames):
            combined_buffers(data, nframestotal), sizeOfOneChannelInFrames(sizeOfOneChannelInFrames){}
        
        PlanarSampleBufferInlineView(){};
        
        auto operator[](size_t i){
            return channel_at(i);
        }
        const auto operator[](size_t i) const{
            return channel_at(i);
        }
        uint8_t GetNChannels() const{
            return combined_buffers.size() / sizeOfOneChannelInFrames;
        }
        /**
         @return the size of the total storage, NOT the size of a single channel
         */
        auto size() const{
            return combined_buffers.size();
        }

        /**
        * Gets the number of samples, regardless of the number of channels. For example, a 4-channel view and a 1-channel view will return the same number. 
        */
        auto GetNumSamples() const {
            return combined_buffers.size() / GetNChannels();
        }
        
        auto data() {
            return combined_buffers.data();
        }
        
        /**
         @return the size of a single channel, in frames
         */
        auto sizeOneChannel() const{
            return sizeOfOneChannelInFrames;
        }

        /**
        * Copy interleaved data into a planar representation
        * @param interleaved the source buffer
        */
        void ImportInterleavedData(InterleavedSampleBufferView interleaved, uint8_t nchannels) {
            sizeOfOneChannelInFrames = interleaved.size() / nchannels;
#pragma omp simd
            for (size_t i = 0; i < interleaved.size(); i++) {
				auto view = channel_at(i % nchannels);
                view[i / nchannels] = interleaved[i];
            }
        }
    };

    inline void AdditiveBlendSamples(InterleavedSampleBufferView A, const InterleavedSampleBufferView B){
        auto bounds = std::min(A.size(),A.size());
#pragma omp simd
        for(decltype(bounds) i = 0; i < bounds; i++){
            A[i] += B[i];
        }
    }
    inline void AdditiveBlendSamples(PlanarSampleBufferInlineView A, const PlanarSampleBufferInlineView B){
#pragma omp simd
        for(uint8_t c = 0; c < std::min(A.GetNChannels(),B.GetNChannels()); c++){
#pragma omp simd
            for(size_t i = 0; i < A.sizeOneChannel();i++){
                A[c][i] += B[c][i];
            }
        }
    }

    class AudioGraphAsset;
    struct AudioGraphComposed{
        using effect_graph_ptr_t = Ref<AudioGraphAsset>;
    private:
        void renderImpl(PlanarSampleBufferInlineView& inputBuffer, PlanarSampleBufferInlineView& scratchBuffer, uint8_t nchannels);
        effect_graph_ptr_t effectGraph;
    public:
        
        void SetGraph(decltype(effectGraph) inGraph){
            effectGraph = inGraph;
        }
        
        auto GetGraph() const{
            return effectGraph;
        }
        
        /**
         Render the graph in-place, using provided memory for scratch space
         @param inputSamples the input buffer to apply the effect graph to. Contents will be modified after this function.
         @param intermediateBuffer memory of equal size to inputSamples to store intermediate data. Assumed to be zero-filled.
         */
        void Render(PlanarSampleBufferInlineView& inputSamples, PlanarSampleBufferInlineView& intermediateBuffer, uint8_t nchannels){
            renderImpl(inputSamples, intermediateBuffer, nchannels);
        }
    };
}
