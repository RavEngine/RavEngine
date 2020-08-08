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
2. Run one of `init-mac.sh` `init-win.sh` `init-linux.sh` if using a custom build system, otherwise use `add_subdirectory` in your `CMakeLists.txt` 
3. Open `RavEngine.xcodeproj` or `RavEngine.sln` and press Build

### Note for Linux users:
You need to have these packages installed: 
 - X11 dev, OpenGL dev, X11-Athena dev, X-toolkit, Xrandr dev.
You also need a C++ compiler.

These can be installed on Fedora Linux with `sudo dnf install libX11-devel mesa-libGL-devel libXaw-devel libXt libXrandr-devel`
