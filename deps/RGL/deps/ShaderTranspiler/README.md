# ShaderTranspiler
A clean and simple C++ library to convert GLSL shaders to HLSL, Metal, and Vulkan. It leverages [glslang](https://github.com/KhronosGroup/glslang), 
[SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross), and [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) to handle cross-compilation of shaders. 

## Supported APIs:
- GLSL -> ESSL (OpenGL ES) 
- GLSL -> HLSL (DirectX)
- GLSL -> DXIL\* (DirectX)
- GLSL -> MSL (Metal, mobile and desktop)
- GLSL -> MSL Binary (macOS host only, requires Xcode to be installed)
- GLSL -> SPIR-V (Vulkan)


## How to use
```cpp
// include the library
#include <ShaderTranspiler/ShaderTranspiler.hpp>
#include <iostream>

using namespace std::filesystem;
using namespace std;

//the library's namespace
using namespace shadert;

int main(){

  // create an instance
  ShaderTranspiler s;

  // Create a CompileTask with the path to your shader and its stage.
  // The path is required because this library supports the OpenGL #include extension
  // For use cases involving code generation, MemoryCompileTask is also provided.
  FileCompileTask task(path("Scene.vert"),ShaderStage::Vertex);

  // configure the compile with an Options object
  Options opt;
  opt.mobile = false; //used for OpenGL ES or Metal iOS
  opt.version = 15;   //stores the major and minor version, for Vulkan 1.5 use 15

  try{
    // call CompileTo and pass the CompileTask and the Options
    CompileResult result = s.CompileTo(task, TargetAPI::Vulkan, opt);

    // not all of these will be populated depending on the target
    cout << result.sourceData << endl;
    cout << result.binaryData << endl;
  }
  catch(exception& e){
    // library will throw on errors
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
```

## How to compile
This library uses CMake. Simply call `add_subdirectory` in your CMakeLists.txt.
```cmake
# ...
add_subdirectory("ShaderTranspiler")
# ...
target_link_libraries("your-program" PRIVATE "ShaderTranspiler")
```
You will need a C++20 compiler to build the library (but not to use it, the header is compatible with C++17).

If you only want to play around with the library, you can use one of the init scripts (`init-mac.sh`, `init-win.sh`) and modify `main.cpp` in the test folder.

\* DXIL support is restricted to Windows hosts by default. To generate DXIL on non-Windows hosts, clone with submodules to get the [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
source code, and set `ST_BUNDLED_DXC` to `1` in your CMake configuration. This will compile DXC from source and use that instead of the compiler that comes with Direct3D for Windows. Because of how big DXC is 
and how long it takes to compile, this feature is disabled by default. 

# Issue reporting
Known issues:
- Writing SPIR-V binaries on a Big Endian machine will not work.

Use the Issues section to report problems. Contributions are welcome, use pull requests. 
