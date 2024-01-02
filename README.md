# RavEngine
A C++20 cross-platform game framework, with emphasis on addressing pain points in existing game engines. Notable features:
1. Fast Parallel ECS, with support for querying by base class
2. Multithreaded physics simulation
3. 3D spatial audio, including room reverbation modeling
4. Automatic memory management handled via reference counting, no garbage collector
5. GPU-driven render engine that supports modern rendering APIs (Metal, DirectX 12, Vulkan)
6. Physically-based lighting model
7. Author shaders in vanilla GLSL
8. Declarative user interface system based on HTML and CSS
9. Support for SVGs in the UI and for textures
10. Built-in multiplayer networking system 
11. FSM animation blending tree system
12. Compute shader mesh skinning with automatic batching
13. Programmable audio effect processing system
14. AR/VR support via OpenXR integration
15. CI/CD-friendly build process powered by CMake
16. Quality-of-life features like automatic incremental shader compilation

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
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_20)  # require C++20

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
RavEngine is sensitive to file extensions with shaders. `vsh`, `fsh`, and `csh` determine if the shader is compiled as vertex, fragment, or compute.

Then build with CMake as normal. On Windows, you will need to run your initial configure twice before building (known bug). Example scripts are provided. 

## Supported platforms
| Platform | Architecture | Compiler | CMake Generator | Rendering API |
| --- | --- | --- | --- | --- |
| macOS 12+ | Intel, Apple Silicon | Apple Clang | Xcode | Metal |
| iOS 16+ | Device + Simulator* | Apple Clang | Xcode | Metal |
| tvOS 16+ | Device + Simulator* | Apple Clang | Xcode | Metal |
| Windows 10+ (Win32) | x86_64, aarch64 | MSVC | Visual Studio, Ninja | DX12, Vulkan |
| Windows 10+ (GDK) | x86_64 | MSVC | Visual Studio | DX12, Vulkan |
| Windows 10+ (UWP) | x86_64, aarch64 | MSVC | Visual Studio | DX12 |
| Xbox Series (UWP) | x86_64 | MSVC | Visual Studio | DX12 |
| Linux | x86_64, aarch64 | Clang, gcc | Ninja, Make | Vulkan |
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

Note that for end users the minimum system requirements are much lower

Software (required)
- macOS, iOS, tvOS
   - Xcode
   - CMake
- Windows, UWP, Xbox
   - Visual Studio 2022 or later with Desktop Development in C++ module
   - CMake (included with Visual Studio)
   - LunarG Vulkan SDK (optional)
- Linux
   - ninja (recommended) or make
   - clang 14 or later (recommended) or gcc 12 or later
   - cmake
   - libatomic
   - x11-dev
   - libwayland-dev
   - uuid-dev
   - alsa-lib-devel or pulseaudio (or another SDL2-supported audio library)
   - [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (required)


## Example programs
View a respository with code samples here: [https://github.com/RavEngine/Samples](https://github.com/RavEngine/Samples)
