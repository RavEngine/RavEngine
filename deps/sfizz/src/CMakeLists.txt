set(FAUST_FILES
    sfizz/dsp/filters/filters_modulable.dsp
    sfizz/dsp/filters/rbj_filters.dsp
    sfizz/dsp/filters/sallenkey_modulable.dsp
    sfizz/dsp/filters/sfz_filters.dsp
    sfizz/effects/dsp/limiter.dsp
    sfizz/effects/dsp/resonant_string.dsp
    sfizz/effects/dsp/compressor.dsp
    sfizz/effects/dsp/gate.dsp
    sfizz/effects/dsp/disto_stage.dsp
    sfizz/effects/dsp/fverb.dsp)
source_group("Faust Files" FILES ${FAUST_FILES})

set(SFIZZ_HEADERS
    sfizz/ADSREnvelope.h
    sfizz/AudioBuffer.h
    sfizz/AudioReader.h
    sfizz/AudioSpan.h
    sfizz/BeatClock.h
    sfizz/Buffer.h
    sfizz/BufferPool.h
    sfizz/CCMap.h
    sfizz/Config.h
    sfizz/Curve.h
    sfizz/utility/Debug.h
    sfizz/utility/LeakDetector.h
    sfizz/utility/Macros.h
    sfizz/utility/MemoryHelpers.h
    sfizz/utility/NumericId.h
    sfizz/utility/StringViewHelpers.h
    sfizz/utility/SwapAndPop.h
    sfizz/utility/Timing.h
    sfizz/utility/XmlHelpers.h
    sfizz/modulations/ModId.h
    sfizz/modulations/ModKey.h
    sfizz/modulations/ModKeyHash.h
    sfizz/modulations/ModMatrix.h
    sfizz/modulations/ModGenerator.h
    sfizz/modulations/sources/ADSREnvelope.h
    sfizz/modulations/sources/ChannelAftertouch.h
    sfizz/modulations/sources/PolyAftertouch.h
    sfizz/modulations/sources/Controller.h
    sfizz/modulations/sources/FlexEnvelope.h
    sfizz/modulations/sources/LFO.h
    sfizz/effects/impl/ResonantArray.h
    sfizz/effects/impl/ResonantArrayAVX.h
    sfizz/effects/impl/ResonantArraySSE.h
    sfizz/effects/impl/ResonantString.h
    sfizz/effects/impl/ResonantStringAVX.h
    sfizz/effects/impl/ResonantStringSSE.h
    sfizz/effects/Apan.h
    sfizz/effects/Compressor.h
    sfizz/effects/Disto.h
    sfizz/effects/Eq.h
    sfizz/effects/Filter.h
    sfizz/effects/Fverb.h
    sfizz/effects/Gain.h
    sfizz/effects/Gate.h
    sfizz/effects/Limiter.h
    sfizz/effects/Lofi.h
    sfizz/effects/Nothing.h
    sfizz/effects/Rectify.h
    sfizz/effects/Strings.h
    sfizz/effects/Width.h
    sfizz/Effects.h
    sfizz/EGDescription.h
    sfizz/EQDescription.h
    sfizz/EQPool.h
    sfizz/FileId.h
    sfizz/FileMetadata.h
    sfizz/FilePool.h
    sfizz/FilterDescription.h
    sfizz/FilterPool.h
    sfizz/FlexEGDescription.h
    sfizz/FlexEnvelope.h
    sfizz/HistoricalBuffer.h
    sfizz/Interpolators.h
    sfizz/Interpolators.hpp
    sfizz/Layer.h
    sfizz/LFO.h
    sfizz/LFOCommon.h
    sfizz/LFOCommon.hpp
    sfizz/LFODescription.h
    sfizz/MathHelpers.h
    sfizz/Metronome.h
    sfizz/MidiState.h
    sfizz/ModifierHelpers.h
    sfizz/OnePoleFilter.h
    sfizz/Oversampler.h
    sfizz/Panning.h
    sfizz/PolyphonyGroup.h
    sfizz/PowerFollower.h
    sfizz/railsback/2-1.h
    sfizz/railsback/4-1.h
    sfizz/railsback/4-2.h
    sfizz/Region.h
    sfizz/RegionStateful.h
    sfizz/RegionSet.h
    sfizz/Resources.h
    sfizz/RTSemaphore.h
    sfizz/ScopedFTZ.h
    sfizz/SfzFilter.h
    sfizz/SfzFilterImpls.hpp
    sfizz/simd/Common.h
    sfizz/simd/HelpersAVX.h
    sfizz/simd/HelpersScalar.h
    sfizz/simd/HelpersSSE.h
    sfizz/SIMDConfig.h
    sfizz/SIMDHelpers.h
    sfizz/SisterVoiceRing.h
    sfizz/Smoothers.h
    sfizz/Synth.h
    sfizz/SynthConfig.h
    sfizz/SynthPrivate.h
    sfizz/Tuning.h
    sfizz/Voice.h
    sfizz/VoiceManager.h
    sfizz/VoiceStealing.h
    sfizz/Wavetables.h
    sfizz/WindowedSinc.h
    sfizz/WindowedSinc.hpp
    sfizz.h
    sfizz.hpp)

set(SFIZZ_SOURCES
    sfizz/Synth.cpp
    sfizz/FileId.cpp
    sfizz/FilePool.cpp
    sfizz/FileMetadata.cpp
    sfizz/AudioReader.cpp
    sfizz/FilterPool.cpp
    sfizz/EQPool.cpp
    sfizz/RegionStateful.cpp
    sfizz/Region.cpp
    sfizz/Voice.cpp
    sfizz/ScopedFTZ.cpp
    sfizz/MidiState.cpp
    sfizz/Oversampler.cpp
    sfizz/ADSREnvelope.cpp
    sfizz/SfzFilter.cpp
    sfizz/Curve.cpp
    sfizz/Smoothers.cpp
    sfizz/Wavetables.cpp
    sfizz/Tuning.cpp
    sfizz/RegionSet.cpp
    sfizz/PolyphonyGroup.cpp
    sfizz/VoiceManager.cpp
    sfizz/VoiceStealing.cpp
    sfizz/RTSemaphore.cpp
    sfizz/Panning.cpp
    sfizz/Effects.cpp
    sfizz/LFO.cpp
    sfizz/LFODescription.cpp
    sfizz/PowerFollower.cpp
    sfizz/FlexEGDescription.cpp
    sfizz/FlexEnvelope.cpp
    sfizz/BeatClock.cpp
    sfizz/Metronome.cpp
    sfizz/SynthMessaging.cpp
    sfizz/WindowedSinc.cpp
    sfizz/Interpolators.cpp
    sfizz/Layer.cpp
    sfizz/Resources.cpp
    sfizz/modulations/ModId.cpp
    sfizz/modulations/ModKey.cpp
    sfizz/modulations/ModKeyHash.cpp
    sfizz/modulations/ModMatrix.cpp
    sfizz/modulations/sources/ADSREnvelope.cpp
    sfizz/modulations/sources/ChannelAftertouch.cpp
    sfizz/modulations/sources/PolyAftertouch.cpp
    sfizz/modulations/sources/Controller.cpp
    sfizz/modulations/sources/FlexEnvelope.cpp
    sfizz/modulations/sources/LFO.cpp
    sfizz/effects/Nothing.cpp
    sfizz/effects/Filter.cpp
    sfizz/effects/Eq.cpp
    sfizz/effects/Apan.cpp
    sfizz/effects/Lofi.cpp
    sfizz/effects/Limiter.cpp
    sfizz/effects/Compressor.cpp
    sfizz/effects/Gate.cpp
    sfizz/effects/Disto.cpp
    sfizz/effects/Strings.cpp
    sfizz/effects/Fverb.cpp
    sfizz/effects/Rectify.cpp
    sfizz/effects/Gain.cpp
    sfizz/effects/Width.cpp
    sfizz/effects/impl/ResonantString.cpp
    sfizz/effects/impl/ResonantStringSSE.cpp
    sfizz/effects/impl/ResonantStringAVX.cpp
    sfizz/effects/impl/ResonantArray.cpp
    sfizz/effects/impl/ResonantArraySSE.cpp
    sfizz/effects/impl/ResonantArrayAVX.cpp
    sfizz/utility/c++17/AlignedMemorySupport.cpp)

include(SfizzSIMDSourceFiles)
sfizz_add_simd_sources(SFIZZ_SOURCES ".")

# Parser core library
set(SFIZZ_PARSER_HEADERS
    sfizz/Defaults.h
    sfizz/Range.h
    sfizz/Opcode.h
    sfizz/parser/Parser.h
    sfizz/parser/ParserListener.h
    sfizz/parser/ParserPrivate.h
    sfizz/parser/ParserPrivate.hpp
    sfizz/SfzHelpers.h)

set(SFIZZ_PARSER_SOURCES
    sfizz/Opcode.cpp
    sfizz/Defaults.cpp
    sfizz/OpcodeCleanup.cpp
    sfizz/parser/Parser.cpp
    sfizz/parser/ParserPrivate.cpp)

set(SFIZZ_PARSER_OTHER sfizz/OpcodeCleanup.re)
source_group("Other Files" FILES ${SFIZZ_PARSER_OTHER})

# Sfizz parser library
add_library(sfizz_parser STATIC)
add_library(sfizz::parser ALIAS sfizz_parser)
target_sources(sfizz_parser PRIVATE
    ${SFIZZ_PARSER_HEADERS} ${SFIZZ_PARSER_SOURCES} ${SFIZZ_PARSER_OTHER})
target_include_directories(sfizz_parser PUBLIC sfizz)
target_link_libraries(sfizz_parser
    PUBLIC sfizz::filesystem sfizz::simde absl::strings
    PRIVATE absl::flat_hash_map)
if(SFIZZ_RELEASE_ASSERTS)
    target_compile_definitions(sfizz_parser PUBLIC "SFIZZ_ENABLE_RELEASE_ASSERT=1")
endif()

# OSC messaging library
set(SFIZZ_MESSAGING_HEADERS
    sfizz/Messaging.h
    sfizz/Messaging.hpp
    sfizz_message.h)

set(SFIZZ_MESSAGING_SOURCES
    sfizz/Messaging.cpp)

add_library(sfizz_messaging STATIC)
add_library(sfizz::messaging ALIAS sfizz_messaging)
target_sources(sfizz_messaging PRIVATE
    ${SFIZZ_MESSAGING_HEADERS} ${SFIZZ_MESSAGING_SOURCES})
target_include_directories(sfizz_messaging PUBLIC ".")
target_link_libraries(sfizz_messaging PUBLIC absl::strings)
if(SFIZZ_RELEASE_ASSERTS)
    target_compile_definitions(sfizz_messaging PUBLIC "SFIZZ_ENABLE_RELEASE_ASSERT=1")
endif()

# Import library
set(SFIZZ_IMPORT_HEADERS
    sfizz/import/sfizz_import.h
    sfizz/import/ForeignInstrument.h
    sfizz/import/foreign_instruments/AudioFile.h
    sfizz/import/foreign_instruments/DecentSampler.h)

set(SFIZZ_IMPORT_SOURCES
    sfizz/import/sfizz_import.cpp
    sfizz/import/ForeignInstrument.cpp
    sfizz/import/foreign_instruments/AudioFile.cpp
    sfizz/import/foreign_instruments/DecentSampler.cpp)

add_library(sfizz_import STATIC)
add_library(sfizz::import ALIAS sfizz_import)
target_sources(sfizz_import PRIVATE
    ${SFIZZ_IMPORT_HEADERS} ${SFIZZ_IMPORT_SOURCES})
target_include_directories(sfizz_import PUBLIC ".")
target_link_libraries(sfizz_import
    PUBLIC absl::strings absl::memory sfizz::filesystem
    PRIVATE sfizz::internal sfizz::pugixml)

# Sfizz spinlock mutex
add_library(sfizz_spin_mutex STATIC
    sfizz/utility/spin_mutex/spin_mutex.h
    sfizz/utility/spin_mutex/spin_mutex.cpp
    sfizz/utility/spin_mutex/SpinMutex.h
    sfizz/utility/spin_mutex/SpinMutex.cpp)
target_include_directories(sfizz_spin_mutex PUBLIC sfizz/utility/spin_mutex)
target_link_libraries(sfizz_spin_mutex PRIVATE sfizz::atomic_queue)
add_library(sfizz::spin_mutex ALIAS sfizz_spin_mutex)

# Sfizz bit array
add_library(sfizz_bit_array INTERFACE)
target_include_directories(sfizz_bit_array INTERFACE sfizz/utility/bit_array)
add_library(sfizz::bit_array ALIAS sfizz_bit_array)

# Sfizz internals (use this for testing)
add_library(sfizz_internal STATIC)
add_library(sfizz::internal ALIAS sfizz_internal)
target_sources(sfizz_internal PRIVATE ${SFIZZ_HEADERS} ${SFIZZ_SOURCES} ${FAUST_FILES})
target_include_directories(sfizz_internal PUBLIC "." "sfizz")
target_link_libraries(sfizz_internal
    PUBLIC absl::strings absl::span sfizz::filesystem sfizz::atomic_queue sfizz::spin_mutex sfizz::bit_array sfizz::simde sfizz::hiir sfizz::jsl
    PRIVATE sfizz::parser sfizz::messaging absl::flat_hash_map Threads::Threads st_audiofile sfizz::pugixml sfizz::spline sfizz::tunings sfizz::kissfft sfizz::cephes sfizz::cpuid sfizz::threadpool sfizz::atomic)
if(SFIZZ_USE_SNDFILE)
    target_compile_definitions(sfizz_internal PUBLIC "SFIZZ_USE_SNDFILE=1")
    target_link_libraries(sfizz_internal PUBLIC st_audiofile)
endif()
if(SFIZZ_RELEASE_ASSERTS)
    target_compile_definitions(sfizz_internal PUBLIC "SFIZZ_ENABLE_RELEASE_ASSERT=1")
endif()
if(SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT)
    target_compile_definitions(sfizz_internal PRIVATE "SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT=1")
endif()
sfizz_enable_fast_math(sfizz_internal)

# Check that sfizz and cmake-side definitions are matching
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sfizz_match_definitions.cpp" "
#include \"Config.h\"
static_assert(sfz::config::numCCs == ${SFIZZ_NUM_CCS}, \"SFIZZ_NUM_CCS did not match\");
")
add_library(sfizz_match_definitions STATIC "${CMAKE_CURRENT_BINARY_DIR}/sfizz_match_definitions.cpp")
target_link_libraries(sfizz_match_definitions PRIVATE sfizz::internal)

# Sfizz static library
add_library(sfizz_static STATIC sfizz/sfizz_wrapper.cpp sfizz/sfizz.cpp sfizz/sfizz_private.hpp)
add_library(sfizz::static ALIAS sfizz_static)
target_include_directories(sfizz_static PUBLIC .)
target_link_libraries(sfizz_static PRIVATE sfizz::internal)
set_target_properties(sfizz_static PROPERTIES PUBLIC_HEADER "sfizz.h;sfizz.hpp;sfizz_message.h")
if(MSVC)
    set_target_properties(sfizz_static PROPERTIES OUTPUT_NAME "sfizz_static")
else()
    set_target_properties(sfizz_static PROPERTIES OUTPUT_NAME "sfizz")
endif()

# Shared library and installation target
if(SFIZZ_SHARED)
    add_library(sfizz_shared SHARED sfizz/sfizz_wrapper.cpp sfizz/sfizz.cpp)
    add_library(sfizz::shared ALIAS sfizz_shared)
    target_include_directories(sfizz_shared PUBLIC .)
    target_link_libraries(sfizz_shared PRIVATE sfizz::internal)
    target_compile_definitions(sfizz_shared PRIVATE SFIZZ_EXPORT_SYMBOLS)
    set_target_properties(sfizz_shared PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}" OUTPUT_NAME "sfizz" PUBLIC_HEADER "sfizz.h;sfizz.hpp;sfizz_message.h")
    sfizz_enable_lto_if_needed(sfizz_shared)

    if(NOT MSVC)
        install(TARGETS sfizz_shared
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT "runtime"
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT "runtime"
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT "development"
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT "development")
        set(SFIZZ_PC_REQUIRES "")
        if(SFIZZ_USE_SNDFILE)
            set(SFIZZ_PC_REQUIRES "${SFIZZ_PC_REQUIRES} sndfile")
        endif()
        configure_file(${PROJECT_SOURCE_DIR}/scripts/sfizz.pc.in sfizz.pc @ONLY)
        install(FILES ${CMAKE_BINARY_DIR}/src/sfizz.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
            COMPONENT "development")
    endif()
endif()

# Generic library alias
add_library(sfizz::sfizz ALIAS sfizz_static)

# Git build identifier
add_custom_target(sfizz-generate-git-build-id
    COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
            "-DOUTPUT_FILE=${PROJECT_BINARY_DIR}/git-build-id/GitBuildId.c"
            "-P" "${PROJECT_SOURCE_DIR}/cmake/GitBuildID.cmake"
    BYPRODUCTS "${PROJECT_BINARY_DIR}/git-build-id/GitBuildId.c")
add_library(sfizz-git-build-id STATIC EXCLUDE_FROM_ALL
    "sfizz/git-build-id/GitBuildId.h"
    "${PROJECT_BINARY_DIR}/git-build-id/GitBuildId.c")
target_include_directories(sfizz-git-build-id PUBLIC "sfizz/git-build-id")
add_dependencies(sfizz-git-build-id sfizz-generate-git-build-id)

# Preserve generated files (Faust)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM TRUE)

# Faust filters
foreach(filter_type
        Lpf1p Lpf2p Lpf4p Lpf6p Hpf1p Hpf2p Hpf4p Hpf6p
        Bpf1p Bpf2p Bpf4p Bpf6p Apf1p Brf1p Brf2p
        Pink Lpf2pSv Hpf2pSv Bpf2pSv Brf2pSv
        Lsh Hsh Peq EqPeak EqLshelf EqHshelf)
    add_faust_command(
        "sfizz/dsp/filters/sfz_filters.dsp"
        "sfizz/gen/filters/sfz${filter_type}.hxx"
        DOUBLE IN_PLACE
        PROCESS_NAME "sfz${filter_type}"
        CLASS_NAME "faust${filter_type}"
        SUPERCLASS_NAME "sfzFilterDsp"
        IMPORT_DIRS "sfizz/dsp")
    add_faust_command(
        "sfizz/dsp/filters/sfz_filters.dsp"
        "sfizz/gen/filters/sfz2ch${filter_type}.hxx"
        DOUBLE IN_PLACE
        PROCESS_NAME "sfz2ch${filter_type}"
        CLASS_NAME "faust2ch${filter_type}"
        SUPERCLASS_NAME "sfzFilterDsp"
        IMPORT_DIRS "sfizz/dsp")
    target_sources(sfizz_internal PRIVATE
        "sfizz/gen/filters/sfz${filter_type}.hxx"
        "sfizz/gen/filters/sfz2ch${filter_type}.hxx")
endforeach()

# Faust effects
add_faust_command(
    "sfizz/effects/dsp/compressor.dsp"
    "sfizz/effects/gen/compressor.hxx"
    IN_PLACE
    CLASS_NAME "faustCompressor"
    IMPORT_DIRS "sfizz/dsp")
add_faust_command(
    "sfizz/effects/dsp/disto_stage.dsp"
    "sfizz/effects/gen/disto_stage.hxx"
    IN_PLACE
    CLASS_NAME "faustDisto"
    IMPORT_DIRS "sfizz/dsp")
add_faust_command(
    "sfizz/effects/dsp/fverb.dsp"
    "sfizz/effects/gen/fverb.hxx"
    IN_PLACE
    CLASS_NAME "faustFverb"
    IMPORT_DIRS "sfizz/dsp")
add_faust_command(
    "sfizz/effects/dsp/gate.dsp"
    "sfizz/effects/gen/gate.hxx"
    IN_PLACE
    CLASS_NAME "faustGate"
    IMPORT_DIRS "sfizz/dsp")
add_faust_command(
    "sfizz/effects/dsp/limiter.dsp"
    "sfizz/effects/gen/limiter.hxx"
    IN_PLACE
    CLASS_NAME "faustLimiter"
    IMPORT_DIRS "sfizz/dsp")
target_sources(sfizz_internal PRIVATE
    "sfizz/effects/gen/compressor.hxx"
    "sfizz/effects/gen/disto_stage.hxx"
    "sfizz/effects/gen/fverb.hxx"
    "sfizz/effects/gen/gate.hxx"
    "sfizz/effects/gen/limiter.hxx")

# Windows installer
if(WIN32)
    #include(VSTConfig)
    configure_file(${PROJECT_SOURCE_DIR}/scripts/innosetup.iss.in ${PROJECT_BINARY_DIR}/innosetup.iss @ONLY)
endif()
