# Building Resonance Audio

To retrieve latest changes from Resonance Audio project, clone from https://github.com/resonance-audio/resonance-audio.git and instructions for basic setup https://github.com/resonance-audio/resonance-audio
Each platform requires api/lowlevel/inc/* as setup in the fmod sdk in order to build the plugin.
* For Windows, this should be located in C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\lowlevel\inc
* for iOS/OSX, this should be located in "<path_to_resonance_audio>/third_party/FMOD Programmers API/api/lowlevel/inc"
* Otherwise, this should be located in "<path_to_resonance_audio>/third_party/fmod/api/lowlevel/inc"
* This folder should be copied into the "third_party" folder of resonance-audio.
Specific instructions for building each version are listed below. There are two separate instruction sets - one utilising the build.sh script and one with manual cmake configuration for when the build script fails.

## Windows

Additional Software requirements:
* Visual Studio 2017
* Git (you will probably run git-bash.exe to install dependencies, so you may not need git to be available via the command line)
* Mercurial - needs to be available on the command line
* Cmake GUI

build.sh guide
* Note: build.sh out of date with current Cmake and configured for Visual Studio 2015, recommend following the manual steps.
* Expects to find fmod headers in "C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\lowlevel\inc"
* Requires git-bash or similar shell environment, run: ./build.sh -t=FMOD_PLUGIN
* In build.sh, you need to change variable MSVC_GENERATOR between "Visual Studio 14 2015 Win64" and "Visual Studio 14 2015" to build both 32 and 64 bit dlls

Manual Steps
* Run cmake-gui.exe and specify both a build directory and the resonance audio directory. Click configure and select Visual Studio 15 2017.
* Click BUILD_FMOD_PLUGIN then click configure again.
* Set the FMOD_SDK_DIR to the third_party/fmod directory and click generate.
* Open the generated solution in Visual Studio, set the configuration to release.
* In the "resonanceaudio.vcxproj", under project settings -> linker settings:
    * Change "Debugging -> Generate Debug Info" to "Generate Debug Information optimized for sharing and publishing (/DEBUG:FULL)"
    * Change "Optimization -> References" to "Yes (/OPT:REF)"
* Build, then copy the dll generated.
* Then repeat the entire process specifying "Optional platform for generator" to x64

Potential issues:
* If the Win32 (should not be a problem x64) projects are generated with a module definition file, this may create a dll with symbol names that differ from what FMOD expects

## Android

Additional Software requirements (Android is built on Windows)
* Visual Studio 2015
* NVidia CodeWorks (include android visual studio support)
* Git (you will probably run git-bash.exe to install dependencies, so you may not need git to be available via the command line)
* Mercurial - needs to be available on the command line
* Cmake - included in NVidia CodeWorks but expected to be located in path

build.sh guide
* Expects to find fmod headers in "<path_to_resonance_audio>\third_party\fmod\api\lowlevel\inc"
* build.sh variables may have to be changed in case defaults do not line up correctly:
    ANDROID_NDK="<path_to_ndk>"
    MSVC_GENERATOR="Visual Studio 14 2015"
* ANDROID_ABI variable can be changed to produce different builds
* If missing or retrieving the script from GitHub, you may need to add the following line to build.sh after: --android_toolchain)
        CONFIG_FLAGS+=(-DCMAKE_ANDROID_API_MIN="${ANDROID_NATIVE_API_LEVEL}")
* Requires git-bash or similar shell environment. From the resonance audio directory, run: ./build.sh -t=FMOD_PLUGIN --android_toolchain
* installed file will not be stripped - you may want the stripped version that is also built at the same time - will be located in <path_to_resonance_audio>\build\platforms\fmod\Release.
* For different builds, change build.sh variable ANDROID_ABI to:
    armeabi-v7a with NEON
    arm64-v8a (note: may not work correctly (may build 32 bit instead) - see manual instructions)
    x86 (note: may not work correctly - see manual instructions)
    x86_64 (note: may not work correctly - see manual instructions)

Manual Steps
* Run cmake from the command line to generate build files in the current directory - 
    cmake <path_to_resonance_audio> -G "Visual Studio 14 2015" -DBUILD_FMOD_PLUGIN=ON -DCMAKE_SYSTEM_NAME=Android -DANDROID=1 -DCMAKE_ANDROID_API=21 -DCMAKE_ANDROID_API_MIN=21

* Open the generated solution file in the specified folder. There are four target architectures to build:

* ARMv7-A (should be default)
* ARMv8-A, 64bit
* x86
* x86_64

* Build each and use stripped version will be located in <path_to_cmake_current_directory>\build\platforms\fmod\Release.
* You will notice the the build produced via build.sh differs by default from the solution built file. This is due to STL Type property on each of the projects being set to "GNU libstdc++ Static"

## Linux

Additional Software requirements (generated on Linux - recommed Windows Subsystem for Linux - https://docs.microsoft.com/en-gb/windows/wsl/install-win10 )
* gcc/g++
* Git
* Mercurial
* Cmake

build.sh guide
* build.sh may expect Unix line endings - make sure to convert the file if you encounter errors
* Expects to find fmod headers in "<path_to_resonance_audio>/third_party/fmod/api/lowlevel/inc"
* Run via terminal: ./build.sh -t=FMOD_PLUGIN

Manual Steps
* Run cmake from the command line to generate build files in the resonance audio directory:
    cmake <path_to_resonance_audio> -DBUILD_FMOD_PLUGIN=ON -DFMOD_SDK_DIR="<path_to_fmod/api/lowlevel/inc/*>"
* From the resonance audio directory, run make
* Output file should be located at <path_to_resonance_audio>/platforms/fmod/libresonanceaudio.so

## OSX

Additional Software requirements
* Xcode
* Git (bundled with OSX)
* Mercurial - may be easiest to install via homebrew if available, otherwise from their website
* Cmake - download from the website as that version includes the GUI. Once installed, for command line run: sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install

build.sh guide
* build.sh may expect Unix line endings - make sure to convert the file if you encounter errors eg -bash: ./build.sh: /bin/bash^M: bad interpreter: No such file or directory
* build.sh may not have executable permissions - run "chmod +x <path_to_build.sh>"
* Expects to find fmod headers in "<path_to_resonance_audio>/third_party/FMOD Programmers API/api/lowlevel/inc"
* Run via terminal: ./build.sh -t=FMOD_PLUGIN

Manual Steps
* Run the cmake gui application and specify both a binaries directory and the resonance audio source directory. Click configure and select Xcode.
* Click BUILD_FMOD_PLUGIN then click configure again
* Set the FMOD_SDK_DIR to the third_party/fmod directory setup in the General instructions and click generate.
* Open the generated solution in Xcode, Product -> Scheme -> Edit Scheme...
* Change the Run Scheme's Build Configuration to Release, then Product -> Build for -> Running.
* Currently manual build recommended as build.sh generates a library with an unacceptable level of performance. To achieve reasonable performance, the following settings were changed via xcode:
    Other C++ Flags - Remove 'std=gnu++11', leave other flags (-DNDEBUG/-fPIC)
    Relax IEEE Compliance - No
    Link-Time Optimization - Monolithic
    Unroll Loops - Yes
    C Language Dialect - c11
    C++ Language Dialect - C++14
    C++ Standard Library - Compiler Default

Potential issues:
* The project ResonanceAudioObj contains two files with the name utils.cc which may prevent the build from compiling as expeceted.
* If you rename one of them to be unique to the project and update the file name in the CMakeLists.txt for the ResonanceAudioObj, that should work around the problem.

## iOS

Additional Software requirements
* Xcode
* Git (bundled with OSX)
* Mercurial - may be easiest to install via homebrew if available, otherwise from their website
* CMake - download from the website as that version includes the GUI. Once installed, for command line run: sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install

build.sh guide
* build.sh may expect Unix line endings - make sure to convert the file if you encounter errors
* Expects to find fmod headers in "<path_to_resonance_audio>/third_party/FMOD Programmers API/api/lowlevel/inc"
* Run via terminal: ./build.sh -t=FMOD_PLUGIN --ios_os_toolchain

Note:
* Currently manual build recommended as build.sh generates a library with an unacceptable level of performance. To achieve reasonable performance, the following settings were changed via xcode:
    Other C++ Flags - Remove 'std=gnu++11', leave other flags (-DNDEBUG/-fPIC)
    Relax IEEE Compliance - No
    Link-Time Optimization - Monolithic
    Unroll Loops - Yes
    C Language Dialect - c11
    C++ Language Dialect - C++14
    C++ Standard Library - Compiler Default
* To build static library instead of dylib, replace:
        add_library(resonanceaudio SHARED ${RESONANCE_AUDIO_FMOD_PLUGIN_SOURCES} $<TARGET_OBJECTS:ResonanceAudioObj>
                $<TARGET_OBJECTS:SadieHrtfsObj>
                $<TARGET_OBJECTS:PffftObj>)
    With:
        if (IOS_DETECTED)
            set(LIB_BUILD_TYPE STATIC)
        else ()
            set(LIB_BUILD_TYPE SHARED)
        endif ()
        
        add_library(resonanceaudio ${LIB_BUILD_TYPE} ${RESONANCE_AUDIO_FMOD_PLUGIN_SOURCES} $<TARGET_OBJECTS:ResonanceAudioObj>
                $<TARGET_OBJECTS:SadieHrtfsObj>
                $<TARGET_OBJECTS:PffftObj>)
    In <path_to_resonance_audio>\platforms\fmod\CMakeLists.txt

Manual Steps
* Requires CMake ios toolchain which can be located here: https://github.com/leetal/ios-cmake
* Run the cmake gui application and specify both a binaries directory and the resonance audio source directory (top 2 lines of cmake gui).
* Click configure and select Xcode and the "Specify toolchain file for cross-compiling" radio button and click continue.
* Select the ios.toolchain.cmake file from the git repository.
* Check BUILD_FMOD_PLUGIN then click configure again
* Set the FMOD_SDK_DIR to the third_party/fmod directory setup in the General instructions and click generate.
* Open the generated solution (located in the binaries directory specified in cmake) in Xcode, Product -> Scheme -> Edit Scheme...
* Change the Run Scheme's Build Configuration for ALL_BUILD to Release, then Product -> Build for -> Running.
* Ensure that the build platform is set to device, not simulator.

Potential issues:
* The project ResonanceAudioObj contains two files with the name utils.cc which may prevent the build from compiling as expeceted.
* If you rename one of them to be unique to the project and update the file name in the CMakeLists.txt for the ResonanceAudioObj, that should work around the problem.
