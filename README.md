# RavEngine
A C++23 cross-platform game framework, with emphasis on addressing pain points in existing game engines. Notable features:
1. Fast Parallel ECS, with support for querying by base class
2. Multithreaded physics simulation
3. 3D spatial audio, including room reverbation modeling
4. Automatic memory management handled via reference counting, no garbage collector
5. GPU-driven render engine that supports modern rendering APIs (Metal, DirectX 12, Vulkan)
6. Fully-programmable GPU particle system, supporting both arbitrary meshes and billboarded sprites
7. Physically-based lighting model
8. Author shaders in vanilla GLSL
9. Declarative user interface system based on HTML and CSS
10. Support for SVGs in the UI and for textures
11. Built-in multiplayer networking system 
12. FSM animation blending tree system
13. Compute shader mesh skinning with automatic batching
14. Programmable audio effect processing system
15. AR/VR support via OpenXR integration
16. CI/CD-friendly build process powered by CMake
17. Quality-of-life features like automatic incremental shader compilation
18. Integrates with GPU debugging tools (Xcode Metal Debugger, RenderDoc, PIX, NSight)

A complete list of third party technologies can be found in the [`deps`](https://github.com/RavEngine/RavEngine/tree/master/deps) folder.

Note: RavEngine does not have a graphical editor.

## This is an early alpha
Expect bugs and frequent breaking changes. Do not use in serious projects. 

## Integrating and building
Use CMake:
```cmake
cmake_minimum_required(VERSION 3.23)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

# set output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

PROJECT(Example_RavEngine_Game)

add_subdirectory("RavEngine") # configure the engine library

# configure your executable like normal
file(GLOB SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")
add_executable("${PROJECT_NAME}" ${SOURCES})
target_link_libraries("${PROJECT_NAME}" PUBLIC "RavEngine" )  # also adds header includes
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_23)  # Set C++ version

# inform engine about your different assets
file(GLOB objects "objects/*.obj" "objects/*.fbx")
file(GLOB textures "textures/*")
file(GLOB shaders "shaders/*.vsh" "shaders/*.fsh" "shaders/*.csh")
file(GLOB fonts "fonts/*.ttf")
file(GLOB sounds "sounds/*.ogg")
file(GLOB uis "${sample_dir}/ui/*.rml" "${sample_dir}/uis/*.rcss")
pack_resources(TARGET "${PROJECT_NAME}" 
   OBJECTS ${objects}
   SHADERS ${shaders}
   TEXTURES ${textures}
   UIS ${uis}
   FONTS ${fonts}
   SOUNDS ${sounds}
)

# fixup macOS / iOS / tvOS bundle
if(APPLE)
INSTALL(CODE 
   "include(BundleUtilities)
   fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/$<CONFIGURATION>/${PROJECT_NAME}.app\" \"\" \"\")
   " 
   COMPONENT Runtime
)
endif()
```
Then build with CMake as normal.
Other notes:
   - Windows: 
     - You will need to run your initial configure twice before building (known bug).
     - If on an older OS (Windows 10), you'll need to clone recursive to get an updated version of DXC.

## Supported platforms
| Platform | Architecture | Compiler | CMake Generator | Rendering API |
| --- | --- | --- | --- | --- |
| macOS 12+ | Intel, Apple Silicon | Apple Clang | Xcode | Metal |
| iOS 16+ | Device + Simulator* | Apple Clang | Xcode | Metal |
| tvOS 16+ | Device + Simulator* | Apple Clang | Xcode | Metal |
| visionOS 2D 1+ (WIP) | Device + Simulator* | Apple Clang | Xcode | Metal |
| Windows 10+ (Win32) | x86_64, aarch64 | MSVC | Visual Studio, Ninja | DX12, Vulkan |
| Windows 10+ (GDK) | x86_64 | MSVC | Visual Studio | DX12, Vulkan |
| Linux | x86_64, aarch64 | Clang, gcc | Ninja, Make | Vulkan |
| Android (WIP) | x86, x86_64, arm64-v8a, arm7a | NDK Clang | Ninja, Make, Android Studio | Vulkan |
| Emscripten (Early WIP) | WebAssembly | emcc | Ninja, Make | WebGPU |

\* Simulator support is build-only. RavEngine uses GPU features that the simulator does not support.

## System Requirements for Developers
Hardware 
| Part | Recommended | Minimum |
| --- | --- | ---|
| CPU | 16 threads | 4 cores |
| RAM | 32 GB | 16 GB |
| GPU | - | Any with DX12, VK 1.3, or Metal 3 |
| Disk | - | SSD w/ 40GB free space |

Note that for end users the minimum system requirements are much lower.

Software
- macOS, iOS, tvOS, visionOS
   - Xcode 15.3+
   - CMake
- Windows
   - Visual Studio 2022 or later with the "Desktop Development in C++" and "Game Development in C++" modules
   - CMake (included with Visual Studio)
   - LunarG Vulkan SDK (optional unless Vulkan support is desired)
- Android
   - Android Studio 4.2 or later with NDK 27 or later
   - Host requirements:
      - macOS host: Xcode 15.3+
	  - Windows Host: Visual Studio 2022 with the "Desktop Development in C++" and "Game Development in C++" modules
      - Linux host: clang 16 or later (recommended) or gcc 13 or later
   - The [SDL Android Builder](https://github.com/Ravbug/sdl-android-builder/) wrapper project
- Web Browsers
   - Latest [emsdk](https://github.com/emscripten-core/emsdk)
   - CMake
   - Host requirements:
      - macOS: Xcode 15.3+
      - Windows: Visual Studio 2022 with the "Desktop Development in C++" and "Game Development in C++" modules
      - Linux: clang 16 or later (recommended) or gcc 13 or later 
- Linux
   - ninja (recommended) or make
   - clang 16 or later (recommended) or gcc 13 or later
   - cmake
   - libatomic
   - libx11-dev
   - libwayland-dev
   - uuid-dev
   - alsa-lib-devel or pulseaudio (or another SDL3-supported audio library)
   - [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)


## Example programs
View a repository with code samples here: [https://github.com/RavEngine/Samples](https://github.com/RavEngine/Samples)
