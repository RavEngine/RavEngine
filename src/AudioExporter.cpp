#include "AudioExporter.hpp"
#include <sfizz/../../external/st_audiofile/thirdparty/dr_libs/dr_wav.h>
#include "AudioPlayer.hpp"

using namespace RavEngine;

void AudioExporter::ExportWavOneShot(Ref<AudioAsset> asset, const Filesystem::Path &outputPath){
    drwav outputFile;
    drwav_data_format outputFormat {};
    outputFormat.container = drwav_container_riff;
    outputFormat.format = DR_WAVE_FORMAT_PCM;
    outputFormat.channels = asset->GetNChanels();
    outputFormat.sampleRate = AudioPlayer::GetSamplesPerSec();
    outputFormat.bitsPerSample = 16;
 
 #if !defined(_WIN32)
     drwav_bool32 outputFileOk = drwav_init_file_write(&outputFile, outputPath.c_str(), &outputFormat, nullptr);
 #else
     drwav_bool32 outputFileOk = drwav_init_file_write_w(&outputFile, outputPath.c_str(), &outputFormat, nullptr);
 #endif
    
    auto sampleCount = asset->GetNumSamples();
    
    std::vector<int16_t> interleavedPcm(sampleCount);
    
    drwav_f32_to_s16(interleavedPcm.data(),asset->GetData(),sampleCount);
    drwav_write_pcm_frames(&outputFile, sampleCount, interleavedPcm.data());
    
    drwav_uninit(&outputFile);
}
