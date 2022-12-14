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
            return decltype(combined_buffers)(combined_buffers.data()+(i*sizeOfOneChannelInFrames),sizeOfOneChannelInFrames);
        }
        
    public:
        PlanarSampleBufferInlineView(const decltype(combined_buffers)& buf, decltype(sizeOfOneChannelInFrames) sizeOfOneChannelInFrames) : combined_buffers(buf), sizeOfOneChannelInFrames(sizeOfOneChannelInFrames){}
        
        auto operator[](size_t i){
            return channel_at(i);
        }
        const auto operator[](size_t i) const{
            return channel_at(i);
        }
        uint8_t GetNChannels() const{
            return combined_buffers.size() / sizeOfOneChannelInFrames;
        }
    };

    inline void AdditiveBlendSamples(InterleavedSampleBufferView A, const InterleavedSampleBufferView B){
        auto bounds = std::min(A.size(),A.size());
#pragma omp simd
        for(decltype(bounds) i = 0; i < bounds; i++){
            A[i] += B[i];
        }
    }

    class AudioGraphAsset;
    struct AudioGraphComposed{
        using effect_graph_ptr_t = Ref<AudioGraphAsset>;
    private:
        void renderImpl(InterleavedSampleBufferView inputBuffer, InterleavedSampleBufferView scratchBuffer, uint8_t nchannels);
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
        void Render(InterleavedSampleBufferView inputSamples, InterleavedSampleBufferView intermediateBuffer, uint8_t nchannels){
            renderImpl(inputSamples, intermediateBuffer, nchannels);
        }
        
        /**
         Render the graph in-place
         @param inputSamples the input buffer to apply the effect graph to. Contents will be modified after this function.
         @note This function uses stack space decided at runtime. Be aware of the potential consequences of this.
         */
        void Render(InterleavedSampleBufferView inputSamples, uint8_t nchannels){
            stackarray(intermediatebuffer, InterleavedSampleBufferView::value_type, inputSamples.size());
            Render(inputSamples,InterleavedSampleBufferView(intermediatebuffer,inputSamples.size()),nchannels);
        }
        
        /**
         Render the graph, provided a user function to generate the input samples
         @param outputBuffer the memory to write the result into. Assumed to be zero-filled.
         @param func user function which will be invoked to provide samples.
         @note This function uses stack space decided at runtime. Be aware of the potential consequences of this.
         */
        template<typename Func_t>
        void Render(InterleavedSampleBufferView outputBuffer, Func_t&& func, uint8_t nchannels){
            func(outputBuffer);
            Render(outputBuffer, nchannels);
        }
        
        /**
         Render the graph, provided a user function to generate the input samples
         @param outputBuffer the memory to write the result into. Assumed to be zero-filled.
         @param intermediateBuffer memory of equal size to inputSamples to store intermediate data. Assumed to be zero-filled.
         @param func user function which will be invoked to provide samples.
         */
        template<typename Func_t>
        void Render(InterleavedSampleBufferView outputBuffer, InterleavedSampleBufferView intermediateBuffer, Func_t&& func, uint8_t nchannels){
            func(outputBuffer);
            Render(outputBuffer,intermediateBuffer, nchannels);
        }
    };
}
