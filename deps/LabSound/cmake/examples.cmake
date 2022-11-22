
set(labsound_examples_src
    "${LABSOUND_ROOT}/examples/src/ExamplesCommon.h"
    "${LABSOUND_ROOT}/examples/src/Examples.hpp"
    "${LABSOUND_ROOT}/examples/src/ExamplesMain.cpp")

add_executable(LabSoundExample  ${labsound_examples_src})

set(CMAKE_CXX_STANDARD 14)

set(proj LabSoundExample)

if(WIN32)
    # Arch AVX is problematic for many users, so disable it until
    # some reasonable strategy (a separate AVX target?) is determined
    #target_compile_options(${proj} PRIVATE /arch:AVX /Zi)
    target_compile_options(${proj} PRIVATE /Zi)
    target_compile_definitions(${proj} PRIVATE __WINDOWS_WASAPI__=1)
    # TODO: These vars are for libniquist and should be set in the find libynquist script.
    target_compile_definitions(${proj} PRIVATE HAVE_STDINT_H=1 HAVE_SINF=1)
elseif(APPLE)
#    if(CMAKE_OSX_SYSROOT MATCHES ".*iphoneos.*")
#        set(DARWIN_LIBS
#            "-framework AudioToolbox"
#            "-framework Accelerate"
#            "-framework CoreAudio")
#    else()
        set(DARWIN_LIBS
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework Accelerate"
            "-framework CoreAudio"
            "-framework Cocoa")
#    endif()
    target_link_libraries(${proj} ${DARWIN_LIBS})
elseif(ANDROID)
    target_compile_options(${proj} PRIVATE -fPIC)
    target_compile_definitions(${proj} PRIVATE USE_KISS_FFT=1)
    target_link_libraries(${proj} OpenSLES)
    if(LABSOUND_JACK)
        target_link_libraries(${proj} jack)
        target_compile_definitions(${proj} PRIVATE __UNIX_JACK__=1)
    endif()
    if(LABSOUND_PULSE)
        target_link_libraries(${proj} pulse pulse-simple)
        target_compile_definitions(${proj} PRIVATE __LINUX_PULSE__=1)
    endif()
    if(LABSOUND_ASOUND)
        target_link_libraries(${proj} asound)
        target_compile_definitions(${proj} PRIVATE __LINUX_ASOUND__=1)
    endif()
    # TODO: These vars are for libnyquist and should be set in the find libynquist script.
    # TODO: libnyquist's loadabc calls getenv and setenv. That's undesirable.
    target_compile_definitions(${proj} PRIVATE HAVE_STDINT_H=1 HAVE_SETENV=1 HAVE_SINF=1)
elseif(UNIX)
    target_link_libraries(${proj} pthread)
    target_compile_options(${proj} PRIVATE -fPIC)
    target_compile_definitions(${proj} PRIVATE USE_KISS_FFT=1)
    if(LABSOUND_JACK)
        target_link_libraries(${proj} jack)
        target_compile_definitions(${proj} PRIVATE __UNIX_JACK__=1)
    endif()
    if(LABSOUND_PULSE)
        target_link_libraries(${proj} pulse pulse-simple)
        target_compile_definitions(${proj} PRIVATE __LINUX_PULSE__=1)
    endif()
    if(LABSOUND_ASOUND)
        target_link_libraries(${proj} asound)
        target_compile_definitions(${proj} PRIVATE __LINUX_ASOUND__=1)
    endif()
    # TODO: These vars are for libnyquist and should be set in the find libynquist script.
    # TODO: libnyquist's loadabc calls getenv and setenv. That's undesirable.
    target_compile_definitions(${proj} PRIVATE HAVE_STDINT_H=1 HAVE_SETENV=1 HAVE_SINF=1)
endif()

if (APPLE)
endif()

target_link_libraries(LabSoundExample LabSound)

set_target_properties(LabSoundExample PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY bin)

target_compile_definitions(${proj} PRIVATE SAMPLE_SRC_DIR="${LABSOUND_ROOT}/assets")

set_property(TARGET LabSoundExample PROPERTY FOLDER "examples")

install(TARGETS LabSoundExample
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)
