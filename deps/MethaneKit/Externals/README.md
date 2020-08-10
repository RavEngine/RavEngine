# Methane Externals
[Methane Kit](https://github.com/egorodet/MethaneKit) external libraries and tools in linked repositories or prebuilt binaries and headers:
- [CMakeModules](https://github.com/rpavlik/cmake-modules) - RPavlik collection of CMake modules
- [DirectXCompiler](https://github.com/microsoft/DirectXShaderCompiler) - DirectX Shader Compiler for compiling HLSL to DXIL on Windows and HLSL to SPIRV on MacOS (prebuilt command line executables and libraries)
  - [D3DCompilerToDXC](https://github.com/microsoft/DirectXShaderCompiler/blob/master/tools/clang/tools/d3dcomp/) - static library from DirectXShaderCompiler providing functions to bridge from d3dcompiler_47 to dxcompiler  
- [GLSLang](https://github.com/KhronosGroup/glslang) - was previously used to compile HLSL to SPIRV bytecode on MacOS, now it is deprecated (prebuilt command line executable)
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) - used to trsanslate SPIRV bytecode to Metal shaders (prebuilt command line executable)
- [Nowide Standalone](https://github.com/nephatrine/nowide-standalone) - used for short-char to wide-char string conversions on Windows
- [CMRC](https://github.com/vector-of-bool/cmrc) - used for embedding shaders and textures into application binary
- [CML: The Configurable Math Library](https://github.com/demianmnave/CML) - header only template vector math used in sample apps
- [CLI11](https://github.com/CLIUtils/CLI11) - header only command line parser for C++11 and beyond
- [D3DX12](https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Libraries/D3DX12) - DirectX 12 helpers library
- [DirectXTex](https://github.com/microsoft/DirectXTex) - DirectX texture processing library
- [Catch2](https://github.com/catchorg/Catch2) - header only unit-testing framework
- [IttApi](https://github.com/intel/ittapi) - Intel Instrumentation and Tracing Technology (ITT) and Just-In-Time (JIT) API
- [STB](https://github.com/nothings/stb) - stb_image header is used for loading JPEG images to textures
- [PerlinNoise](http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/) - improved Perlin Noise implementation by Stefan Gustavson
- [OpenImageIO](https://github.com/OpenImageIO/oiio) - additional library used for loading of wide range of image formats via native loaders (optional dependency)
  - [USD Build Club](https://github.com/vfxpro99/usd-build-club) - used for [Open Image IO build scripts](OpenImageIO/build) with some modifications
- [Sonar Scanner](https://sonarcloud.io) - scanner binaries for static code analysis via [Scanner Cloud](https://sonarcloud.io/dashboard?id=egorodet_MethaneKit) web-service