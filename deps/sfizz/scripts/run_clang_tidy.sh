#!/bin/sh

clang-tidy \
  src/sfizz/ADSREnvelope.cpp \
  src/sfizz/Curve.cpp \
  src/sfizz/Effects.cpp \
  src/sfizz/EQPool.cpp \
  src/sfizz/FilePool.cpp \
  src/sfizz/FilterPool.cpp \
  src/sfizz/MidiState.cpp \
  src/sfizz/Opcode.cpp \
  src/sfizz/Oversampler.cpp \
  src/sfizz/Panning.cpp \
  src/sfizz/sfizz.cpp \
  src/sfizz/Region.cpp \
  src/sfizz/RegionStateful.cpp \
  src/sfizz/SIMDHelpers.cpp \
  src/sfizz/simd/HelpersSSE.cpp \
  src/sfizz/simd/HelpersAVX.cpp \
  src/sfizz/Synth.cpp \
  src/sfizz/Voice.cpp \
  src/sfizz/effects/Eq.cpp \
  src/sfizz/effects/Filter.cpp \
  src/sfizz/effects/Lofi.cpp \
  src/sfizz/effects/Nothing.cpp \
  plugins/vst/SfizzVstController.cpp \
  plugins/vst/SfizzVstProcessor.cpp \
  plugins/vst/SfizzVstEditor.cpp \
  plugins/vst/SfizzVstState.cpp \
  -- -Iexternal/abseil-cpp -Iexternal/jsl/include -Iexternal/filesystem/include -Iexternal/atomic_queue/include -Iexternal/threadpool -Isrc/external/hiir -Isrc/external/pugixml/src \
     -Iexternal/st_audiofile/src -Iexternal/st_audiofile/thirdparty/dr_libs \
      -Isrc/sfizz -Isrc -Isrc/sfizz/utility/bit_array -Isrc/sfizz/utility/spin_mutex -Isrc/external/spline -Isrc/external/cpuid/src -Iexternal/simde \
      -Iplugins/common -Iplugins/vst -Iplugins/vst/external/VST_SDK/VST3_SDK -Iplugins/editor/external/vstgui4 -Iplugins/vst/external/ring_buffer \
      -Iplugins/editor/src \
      -DNDEBUG -std=c++17
