#include "AudioMIDI.hpp"
#include <sfizz.hpp>
#include "Common3D.hpp"
#include "AudioPlayer.hpp"
#include "AudioRoom.hpp"
#include <sfizz/MathHelpers.h>
#include <sfizz/SfzHelpers.h>
#include <sfizz/SIMDHelpers.h>
#include "App.hpp"
#include "VirtualFilesystem.hpp"

#include <cstdint>

using namespace RavEngine;


namespace midi {
    constexpr uint8_t statusMask { 0b11110000 };
    constexpr uint8_t channelMask { 0b00001111 };
    constexpr uint8_t noteOff { 0x80 };
    constexpr uint8_t noteOn { 0x90 };
    constexpr uint8_t polyphonicPressure { 0xA0 };
    constexpr uint8_t controlChange { 0xB0 };
    constexpr uint8_t programChange { 0xC0 };
    constexpr uint8_t channelPressure { 0xD0 };
    constexpr uint8_t pitchBend { 0xE0 };
    constexpr uint8_t systemMessage { 0xF0 };

    constexpr uint8_t status(uint8_t midiStatusByte)
    {
        return midiStatusByte & statusMask;
    }
    constexpr uint8_t channel(uint8_t midiStatusByte)
    {
        return midiStatusByte & channelMask;
    }

    constexpr int buildAndCenterPitch(uint8_t firstByte, uint8_t secondByte)
    {
        return (int)(((unsigned int)secondByte << 7) + (unsigned int)firstByte) - 8192;
    }
}

void AudioMIDIPlayer::Reset(){
    fmidi_player_rewind(midiPlayer.get());
}

void AudioMIDIPlayer::processEvent(const fmidi_event_t *event, fmidi_seq_event_t *fulldata){
    if (event->type != fmidi_event_type::fmidi_event_message)
        return;
    
    uint16_t track = fulldata->track;
    
    auto& instrument = instrumentTrackMap.at(track).instrument->synthesizer;

    switch (midi::status(event->data[0])) {
        case midi::noteOff:
            instrument.noteOff(delay, event->data[1], event->data[2]);
            break;
        case midi::noteOn:
            if (event->data[2] == 0)
                instrument.noteOff(delay, event->data[1], event->data[2]);
            else
                instrument.noteOn(delay, event->data[1], event->data[2]);
            break;
        case midi::polyphonicPressure:
            break;
        case midi::controlChange:
            instrument.cc(delay, event->data[1], event->data[2]);
            break;
        case midi::programChange:
            break;
        case midi::channelPressure:
            break;
        case midi::pitchBend:
            instrument.pitchWheel(delay, midi::buildAndCenterPitch(event->data[1], event->data[2]));
            break;
        case midi::systemMessage:
            break;
        }
}

void midiTickCallback(const fmidi_event_t * event, void * cbdata, fmidi_seq_event_t* fulldata)
{
    auto data = reinterpret_cast<AudioMIDIPlayer*>(cbdata);
    data->processEvent(event, fulldata);
}

//void AudioMIDIPlayer::EnqueueEvent(const MidiEvent &evt, uint16_t track){
//    instrumentTrackMap.at(track).events.push(evt);
//}
//
//void AudioMIDIPlayer::Render(buffer_t out_buffer){
//    // pop events off and give to the proper instrument
//    const auto samplesPerSec = AudioPlayer::GetSamplesPerSec();
//    
//    float* scratchbuffer[]{new float[out_buffer.size()], new float[out_buffer.size()]};
//
//    for (auto& instrument : instrumentTrackMap){
//        // set the instrument's callback function
//        instrument.instrument->synthesizer.setSampleRate(samplesPerSec);
//        instrument.instrument->synthesizer.enableFreeWheeling();    //TODO: restore old state after processing is finished
//        
//        while (!instrument.events.empty()){
//            // get the next event (without popping it)
//            auto& nextEvent = instrument.events.top();
//            
//            // convert the start tick time into a buffer index
//            // I had help with these equations
//            auto seconds = (nextEvent.tick) * (1.0/ticksPerQuarterNote) * (1.0/beatsPerMinute) * 60;
//            auto bufferidx = size_t(seconds * AudioPlayer::GetSamplesPerSec()) - playhead;
//            
//            // is this event's start point within the buffer?
//            if (bufferidx < out_buffer.size()){
//                // if so, pop it and provide it to the Instrument
//                if (nextEvent.isNoteOn()){
//                    instrument.instrument->synthesizer.noteOn(bufferidx, nextEvent.getKeyNumber(), nextEvent.getVelocity());
//                }
//                else if (nextEvent.isNoteOff()){
//                    instrument.instrument->synthesizer.noteOff(bufferidx, nextEvent.getKeyNumber(), nextEvent.getVelocity());
//                }
//                
//                // consume the event
//                instrument.events.pop();
//            }
//            else{
//                // otherwise, stop the loop
//                break;
//            }
//        }
//         instrument.instrument->synthesizer.renderBlock(scratchbuffer, out_buffer.size(),1);
//    }
//    // advance playhead, now that this buffer processing has completed
//    playhead += out_buffer.size();
//    delete[] scratchbuffer[0];
//    delete[] scratchbuffer[1];
//}

void finishedCallback(void * cbdata)
{
    auto data = reinterpret_cast<AudioMIDIPlayer*>(cbdata);
    data->finishedCurrent = true;
}

void AudioMIDIPlayer::SetMidi(const decltype(midiSMF)& midiFile){
    midiSMF = midiFile;
    midiPlayer = fmidi_player_u{std::move(fmidi_player_new(midiFile.get()))};
    fmidi_player_event_callback(midiPlayer.get(), &midiTickCallback, this);
    fmidi_player_finish_callback(midiPlayer.get(), &finishedCallback, this);
    finishedCurrent = false;
}

void InstrumentSynth::Render(float** scratchBuffer, size_t size, PlanarSampleBufferInlineView output, uint8_t nchannels){
    //TODO: respect nchannels
    synthesizer.renderBlock(scratchBuffer, size);
    PlanarSampleBufferInlineView proc_input(scratchBuffer[0],size,size);    // TODO: this hardcodes to mono
    stackarray(intermediatebuffer, decltype(output)::value_type, output.size());
    PlanarSampleBufferInlineView intermediateView(intermediatebuffer, output.size(), output.size());
    AudioGraphComposed::Render(proc_input, intermediateView, nchannels);
    AdditiveBlendSamples(output, proc_input);
}

void AudioMIDIPlayer::RenderMonoBuffer1024OrLess(PlanarSampleBufferInlineView& out_buffer, PlanarSampleBufferInlineView& effectScratchBuffer){
    
    if (!isPlaying){
        return;
    }
    
    unsigned sampleRate { AudioPlayer::GetSamplesPerSec() };
    auto sampleRateDouble = static_cast<double>(sampleRate);
    const double increment { 1.0 / sampleRateDouble };
    
    // tick player for the size of the buffer
    for (delay = 0; delay < out_buffer.size() && !finishedCurrent; delay++)
        fmidi_player_tick(midiPlayer.get(), increment);
    
    using vtype = std::remove_reference_t<decltype(out_buffer)>::value_type;

    // this might be a bad idea...
    vtype* buffers[]{effectScratchBuffer[0].data(), effectScratchBuffer[0].data() };  // initialize both channels to the same buffer
    
    // render all the instruments and then add into the out_buffer
    for(auto& instrument : instrumentTrackMap){
        instrument.instrument->Render(buffers, out_buffer.size(), out_buffer, 1);
    }
    
    // apply any effect graphs
    // re-use tempbufferL
    AudioGraphComposed::Render(out_buffer, effectScratchBuffer,1);
}

void AudioMIDIPlayer::RenderMono(PlanarSampleBufferInlineView& out_buffer, PlanarSampleBufferInlineView& effectScratchBuffer){
    uint32_t blockSize = std::min<decltype(blockSize)>(1024,AudioPlayer::GetBufferSize());
    // treat the buffer as though it were mono even if it has additional space
    uint64_t next = blockSize;
	auto maxsamples = out_buffer.size() / out_buffer.GetNChannels() ;
    for(uint64_t numFramesWritten { 0 };  numFramesWritten < maxsamples && !finishedCurrent; numFramesWritten += next){
        // slide the view
        PlanarSampleBufferInlineView out_buffer_subset(out_buffer[0].data() + numFramesWritten, next, next);
        PlanarSampleBufferInlineView scratch_buffer_subset(effectScratchBuffer[0].data() + numFramesWritten, next, next);
        RenderMonoBuffer1024OrLess(out_buffer_subset, scratch_buffer_subset);
        next = std::min<size_t>(blockSize, out_buffer.size() - numFramesWritten);
    }
}

void AudioMIDIPlayer::SetInstrumentForTrack(uint16_t track, std::shared_ptr<InstrumentSynth>& instrument){
    if (instrumentTrackMap.size() <= track){
        instrumentTrackMap.resize(closest_multiple_of(track+1,2));
    }
    instrumentTrackMap[track].instrument = instrument;
}

Ref<AudioAsset> AudioMIDIRenderer::Render(const Ref<fmidi_smf_t>& file, AudioMIDIPlayer& player){
    const auto duration = fmidi_smf_compute_duration(file.get());
    player.SetMidi(file);
    player.Play();
    
    const size_t totalSamples = duration * AudioPlayer::GetSamplesPerSec();
    auto assetData = new float[totalSamples]{0};
    std::vector<float> effectData(totalSamples);
    PlanarSampleBufferInlineView assetView(assetData, totalSamples, totalSamples);
    PlanarSampleBufferInlineView effectView(effectData.data(), totalSamples, totalSamples);
    player.RenderMono(assetView, effectView);
  
    auto asset = New<AudioAsset>(InterleavedSampleBufferView{ assetData,totalSamples }, 1);
    return asset;
}

InstrumentSynth::InstrumentSynth(const Filesystem::Path& pathOnDisk, bool notStreaming) {
    if (notStreaming){
        synthesizer.loadSfzFile(pathOnDisk.string());
    }
    else{
        auto rootpath = GetApp()->GetResources().GetStreamingAssetFullRootPath() / pathOnDisk;
        synthesizer.loadSfzFile(rootpath.string());
    }
    synthesizer.setSampleRate(AudioPlayer::GetSamplesPerSec());
    uint32_t blockSize = std::min<decltype(blockSize)>(1024,AudioPlayer::GetBufferSize());
    synthesizer.setSamplesPerBlock(blockSize);
}
