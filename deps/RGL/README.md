# RGL
RavEngine Graphics Library - a thin RHI for Vulkan, DirectX12, and Metal

### Features
- Write shaders in Vanilla GLSL - no macro hacks or custom DSL
- Transpiled shaders retain information in debug, for easier shader source debugging
- Easy-to-understand Metal-like API that maintains the conventions of modern graphics APIs
- Low overhead
  - most calls map directly to their platform-specific counterparts
  - If only one backend is enabled (default for Apple platforms), all virtual function calls dissapear
- Runtime shader compilation via compiler library
- Automatically-managed resource barriers (when using Vulkan or DX12)

### What it is not
- A game engine. See [RavEngine](https://github.com/RavEngine/RavEngine) if that is what you are looking for.
- A windowing / input library. See the code samples for an example that integrates with SDL2.
- Production-ready. Use at your own risk.
- For beginners. Users should be familiar with at least one other modern graphics API. 

### Supported Platforms
- macOS (intel + Apple silicon)
- iOS (device + simulator)
- tvOS (device + simulator)
- Windows 10+ (arm + x86_64)
- Linux
- UWP

### Getting Started
See [RavEngine/RGL-Samples](https://github.com/RavEngine/RGL-Samples) for code samples. To compile the library, use CMake:
```cmake
# ...
add_subdirectory(path/to/RGL)
# ...
target_link_libraries(your_program PUBLIC rgl)

```
