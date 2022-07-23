# RavEngine
A C++17 cross-platform game library that focuses mainly on performance and ease of use.

## Notable features
1. Fast Parallel ECS
   - Unique feature: Supports querying by base classes without vtable!
   - Also supports Unity-style scripting with full automatic parallelization
2. Multithreaded physics simulation (Nvidia PhysX 4.1)
3. 3D spatialized audio with accurate room reverbation modeling (Google Resonance Audio)
4. Automatic memory management handled via reference counting 
5. Supports modern rendering APIs (Metal, DirectX, Vulkan)
6. Flexible and fast declarative user interface system based on HTML and CSS (RmlUi)
7. Support for SVGs in the UI and for textures
8. High-performance easy-to-use multiplayer networking system (Valve GameNetworkingSockets)
9. Full FSM animation blending tree system
10. Compute shader mesh skinning with automatic batching
11. CI/CD-friendly build process powered by CMake
12. Quality-of-life features like automatic incremental shader compilation

Note: RavEngine does not have a graphical editor.

## Version
This project is still in the early alpha stage, with bugs and frequently breaking changes. Please beware and use it cautiously.

## Integrating and building
Integrating this library with CMake is easy. 
```cmake
cmake_minimum_required(VERSION 3.17)

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
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_17)  # require C++17

# inform engine about your different assets
file(GLOB objects "objects/*.obj" "objects/*.fbx")
file(GLOB textures "textures/*")
file(GLOB shaders "shaders/*.cmake")
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
You need to declare your shaders, so that RavEngine can automatically compile them to the correct backend. Create a `.cmake` file and invoke RavEngine's macro:
```cmake
declare_shader("shaderName" "${CMAKE_CURRENT_LIST_DIR}/vertexshader.glsl" "${CMAKE_CURRENT_LIST_DIR}/fragmentshader.glsl" "${CMAKE_CURRENT_LIST_DIR}/varying.def.hlsl")
```
When you load a shader, RavEngine will use the name you specify as the first parameter. To learn how to write BGFX shaders, see documentation at [https://github.com/bkaradzic/bgfx](https://github.com/bkaradzic/bgfx)

Then build with CMake as normal. On Windows, you will need to run your initial configure twice before building. Example scripts are provided. 

## Supported platforms
| Platform | Architecture | Compiler | CMake Generator | Rendering API |
| --- | --- | --- | --- | --- |
| macOS 10.15+ | Intel, Apple Silicon | Apple Clang | Xcode | Metal |
| iOS 14+ | Device + Simulator | Apple Clang | Xcode | Metal |
| tvOS 14+ | Device + Simulator | Apple Clang | Xcode | Metal |
| Windows 10 (Win32) | x86_64, aarch64 | MSVC | Visual Studio | DX12, Vulkan |
| Windows 10 (UWP) | x86_64 aarch64 | MSVC | Visual Studio | DX12 |
| Linux | x86_64, aarch64 | Clang, GCC | Ninja, Make | Vulkan |
| Raspi | aarch64 | Clang, GCC | Ninja, Make | Vulkan |

Note for Linux users: You must have the following shared libaries installed on your system:
- libatomic
- x11-dev, libgl-dev (for X11 support)
- wayland-devel, libxkbcommon-devel, libegl-dev (for Wayland support, note that Wayland is currently not fully supported)
- alsa-lib-devel (aka libasound2-devel) (or another SDL2-supported audio library)

## Example programs
View a respository with code samples here: [https://github.com/ravbug/ravengine-samples](https://github.com/ravbug/ravengine-samples)
