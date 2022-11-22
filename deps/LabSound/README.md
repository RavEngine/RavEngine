

![macOS](https://github.com/LabSound/LabSound/actions/workflows/cmake-macos.yml/badge.svg)
![iOS](https://github.com/LabSound/LabSound/actions/workflows/cmake-ios.yml/badge.svg)
![Windows](https://github.com/LabSound/LabSound/actions/workflows/cmake-windows.yml/badge.svg)
![Linux](https://github.com/LabSound/LabSound/actions/workflows/cmake-ubuntu.yml/badge.svg)
![Android](https://github.com/LabSound/LabSound/actions/workflows/cmake-android.yml/badge.svg)

<p align="center">
  <img src="https://raw.githubusercontent.com/LabSound/LabSound/master/assets/images/labsound_4x3.png"/>
</p>

LabSound is a C++ graph-based audio engine. LabSound originated as a fork of WebKit's WebAudio implementation, as used in Google's Chrome and Apple's Safari.

LabSound implements many aspects of the WebAudio specification while extending its functionality with an improved API, new graph nodes, bugfixes, and performance improvements.

The engine is packaged as a batteries-included static library meant for integration in many types of software: games, visualizers, interactive installations, live coding environments, VST plugins, audio editing/sequencing applications, and more.

[LabSound homepage.](http://www.labsound.io/)

# Features

* Compatibility with the [WebAudio API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API)
* Audio asset loading via [libnyquist](https://github.com/ddiakopoulos/libnyquist)
* Binaural audio via IRCAM HRTF database
* Additional effect & generator nodes (ADSR, noise, stereo delay, and more)
* Compile-time arbitrary function node for DSP processing
* Basic signal analysis nodes (time & frequency)
* Input node (microphone)
* Non-realtime graph processing & wav export
* Thread safety model for multi-threaded apps (e.g. gui)
* SIMD-accelerated channel mixing routines

# Platforms

LabSound supports a variety of backends, including RtAudio and CoreAudio.

LabSound is currently tested on

* Windows 10
* macOS 10.10 to current
* Ubuntu 18.04, using the ALSA and Pulse back ends.

In the past, LabSound has been demonstrated to work on iOS, Android, and Linux via JACK.

# Building

Users of LabSound are expected to compile LabSound from source. While most dependencies are included as code in the repository, libnyquist is bundled as a git submodule so it is required that new users clone the repository with the `--recursive` option.

The submodules can be fetched after a clone with `git submodule update --init --recursive`

LabSound and libnyquist require a C++14 or greater compiler.

# Building with Cmake

LabSound has a `CMakeLists.txt` at the root directory, and all the associated CMake files are in the `cmake/` subfolder. In the following examples, the build is done in a nested build directory. This is not required, it's shown for illustrative purposes. You may put the build and the install prefix anywhere you like.

On macOs and Windows:

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../labsound-distro ..
cmake --build . --target install --config Release
```

On Linux, a backend must be selected, using one of ALSA, Pulse, or Jack. To build with ALSA:

```sh
mkdir build
cd build
cmake -DLABSOUND_ASOUND=1 ..
cmake --build . --target install --config Release
```

On Raspberry Pi, as on Linux, a backend must be selected. The currently tested configuration is ALSA. Be sure to install the asound development libraries before building.

```sh
sudo apt install libasound2-dev
```

On iOS:

```sh
mkdir build
cd build
cmake -G "Xcode" -DCMAKE_TOOLCHAIN_FILE=../cmake/ios-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../labsound-distro-ios ..
cmake --build . --target install --config Release
```

# Examples

LabSound is bundled with many samples. Project files can be found in the `examples/` subfolder.

# Using the Library

Users should link against `liblabsound.a` on OSX and `labsound.lib` on Windows. LabSound also requires symbols from libnyquist, although both the Visual Studio solution and the XCode workspace will build this dependency alongside the core library.

On OSX, new applications also require the following frameworks:
+ Cocoa
+ Accelerate
+ CoreAudio
+ AudioUnit
+ AudioToolbox
+ libnyquist.a

On Windows, new applications also require the following libraries:
+ dsound.lib
+ dxguid.lib
+ winmm.lib
+ libnyquist.lib

For convenience, `LabSound.h` is used as an index header file with all public nodes included for easy application development.

LabSound includes an HRTF implementation. This creates an additional dependency on a folder of impulse wav files when a `PannerNode` is configured to use `PanningMode::HRTF`. The constructor of `PannerNode` will take an additional path to the sample directory relative to the current working directory.

# WebAudio Compatibility

LabSound is derived from one of the original WebAudio implementations, but does not maintain full compatibility with the [spec](http://www.w3.org/TR/webaudio/). In many cases, LabSound has deliberately deviated from the spec for performance or API usability reasons. This is expected to continue into the future as new functionality is added to the engine. It possible to reformulate most WebAudio API sample code written in JS as a LabSound sketch (modulo obvious architectual considerations of JavaScript vs C++).

# License

LabSound is released under the simplified BSD 2 clause license. All LabSound dependencies are under similar permissive licenses. Further details are located in the `LICENSE` and `COPYING` files.
