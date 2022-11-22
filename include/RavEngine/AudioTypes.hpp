#pragma once
#include <span>
#include "Ref.hpp"

namespace RavEngine{
    class AudioAsset;

    /**
     Represents audio samples in interleaved float format. For example, dual-channel audio would be represented
     as LRLRLRLR....
     */
    using InterleavedSampleBuffer = std::span<float,std::dynamic_extent>;

    class AudioGraph;
    struct AudioGraphComposed{
        Ref<AudioGraph> effectGraph;
    };
}
