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
    using InterleavedSampleBuffer = std::span<float,std::dynamic_extent>;

    /**
     Represents audio samples in planar (separated buffers) format.
     Each item in the vector represents a single channels's worth of audio samples
     */
    class PlanarSampleBuffer{
        std::vector<std::span<float,std::dynamic_extent>> buffers;
    public:
        PlanarSampleBuffer(uint8_t nchannels) : buffers(nchannels){}
        auto& operator[](size_t i)
        {
            return buffers[i];
        }
        const auto& operator[](size_t i) const{
            return buffers[i];
        }
    };


    inline void AdditiveBlendSamples(InterleavedSampleBuffer A, const InterleavedSampleBuffer B){
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
        void renderImpl(InterleavedSampleBuffer inputBuffer, InterleavedSampleBuffer scratchBuffer, uint8_t nchannels);
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
        void Render(InterleavedSampleBuffer inputSamples, InterleavedSampleBuffer intermediateBuffer, uint8_t nchannels){
            renderImpl(inputSamples, intermediateBuffer, nchannels);
        }
        
        /**
         Render the graph in-place
         @param inputSamples the input buffer to apply the effect graph to. Contents will be modified after this function.
         @note This function uses stack space decided at runtime. Be aware of the potential consequences of this.
         */
        void Render(InterleavedSampleBuffer inputSamples, uint8_t nchannels){
            stackarray(intermediatebuffer, InterleavedSampleBuffer::value_type, inputSamples.size());
            Render(inputSamples,InterleavedSampleBuffer(intermediatebuffer,inputSamples.size()),nchannels);
        }
        
        /**
         Render the graph, provided a user function to generate the input samples
         @param outputBuffer the memory to write the result into. Assumed to be zero-filled.
         @param func user function which will be invoked to provide samples.
         @note This function uses stack space decided at runtime. Be aware of the potential consequences of this.
         */
        template<typename Func_t>
        void Render(InterleavedSampleBuffer outputBuffer, Func_t&& func, uint8_t nchannels){
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
        void Render(InterleavedSampleBuffer outputBuffer, InterleavedSampleBuffer intermediateBuffer, Func_t&& func, uint8_t nchannels){
            func(outputBuffer);
            Render(outputBuffer,intermediateBuffer, nchannels);
        }
    };
}
