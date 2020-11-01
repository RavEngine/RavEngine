# RavEngine
A C++ cross-platform game framework, with emphasis on performance and ease of use. Notable features:
1. Automatic multithreaded object evaluation 
   - ECS data-oriented and Scripting-style object-oriented supported simultaneously
   - the OOP scripting system is powered by ECS and automatically threaded
2. CPU-multithreaded physics simulation (Nvidia PhysX 4.1)
3. Easy memory management handled via automatic reference counting 
4. Supports native platform rendering APIs (Metal, DirectX)
5. Easy integration with CMake

Note: RavEngine does not have a GUI editor.

## This is early alpha
Expect bugs and frequent breaking changes. Do not use in serious projects. 

## Supported platforms
1. macOS 10.15 or newer (no ARM currently)
2. Windows 10 64 bit (no ARM currently)

## Setup instructions
1. Clone repository (use --depth=1)
2. Run one of `init-mac.sh` `init-win.sh` `init-linux.sh` if using a custom build system, otherwise use `add_subdirectory` in your `CMakeLists.txt` 
3. Open `RavEngine.xcodeproj` or `RavEngine.sln` and press Build

