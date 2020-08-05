# RavEngine
A C++ cross-platform game framework. Notable features
1. multithreaded object evaluation 
   - ECS data-oriented and Actor-Tick object-oriented supported simultaneously
2. GPU-accelerated or CPU-multithreaded physics (Nvidia PhysX)
3. Supports native platform rendering APIs (Metal, DirectX)
4. Easy integration with CMake

## Supported platforms
1. macOS 10.15 or newer (no ARM currently)
2. Windows 10 64 bit (no ARM currently)

## Setup instructions
1. Clone repository (use --depth=1)
2. Run `init-mac.sh` or `init-win.sh` if using a custom build system, otherwise use `add_subdirectory` in your `CMakeLists.txt`
3. Open `RavEngine.xcodeproj` or `RavEngine.sln` and press Build


