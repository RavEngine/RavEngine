#pragma once
#include "Ref.hpp"
#include "Filesystem.hpp"

namespace RavEngine {
struct AudioAsset;

struct AudioExporter{
    /**
     Write an AudioAsset to disk in one-shot as a wav file
     @param asset the asset to write
     @param destination where to write it
     */
    static void ExportWavOneShot(Ref<AudioAsset> asset, const Filesystem::Path& destination);
};

}
