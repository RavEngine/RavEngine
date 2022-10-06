# This script deletes items in subprojects that are not used by RavEngine, such as tests and doc files
# Run this script after incorporating an update for a subproject

# PhysX
rm -r physx/physx/documentation/ physx/physx/samples/ physx/externals/cg-linux/lib physx/externals/cg-linux/bin physx/externals/glew-linux/lib physx/externals/glew/lib physx/externals/opengl/lib64 physx/kaplademo physx/physx/bin physx/externals/clang-physxmetadata physx/externals/opengl-linux physx/externals/glew-linux physx/externals/vswhere

# BGFX, BX
rm -r bgfx.cmake/bgfx/examples bgfx.cmake/bgfx/bindings bgfx.cmake/bgfx/makefile bgfx.cmake/bx/tools

# SDL
rm -r SDL2/test SDL2/Xcode SDL2/Xcode-iOS

# assimp
rm -r assimp/samples assimp/test assimp/doc assimp/tools assimp/scripts

# recast
rm -r recastnavigation/RecastDemo recastnavigation/Tests

# taskflow
rm -r taskflow/3rd-party taskflow/benchmarks taskflow/docs taskflow/doxygen taskflow/examples taskflow/image taskflow/legacy taskflow/sandbox taskflow/unittests taskflow/tfprof

# concurrentqueue
rm -r concurrentqueue/benchmarks concurrentqueue/build concurrentqueue/c_api concurrentqueue/tests

# rmlui
rm -r RmlUI-freetype/RmlUi/Samples RmlUI-freetype/RmlUi/Tests RmlUI-freetype/freetype/docs RmlUI-freetype/freetype/objs RmlUI-freetype/freetype/Makefile RmlUI-freetype/freetype/meson_options.txt RmlUI-freetype/freetype/meson.build RmlUI-freetype/freetype/ChangeLog* RmlUI-freetype/freetype/configure RmlUI-freetype/freetype/autogen.sh
# fmt
rm -r fmt/doc fmt/test

# resonance-audio
rm -r resonance-audio/docs resonance-audio/matlab resonance-audio/platforms/unity resonance-audio/third_party/eigen/doc resonance-audio/third_party/googletest resonance-audio/third_party/eigen/bench resonance-audio/third_party/eigen/test resonance-audio/third_party/eigen/unsupported


# libnyquist
rm -r libnyquist/examples libnyquist/test_data

# GameNetworkingSockets & deps
rm -r GameNetworkingSockets/openssl-cmake/test GameNetworkingSockets/openssl-cmake/doc
rm -r GameNetworkingSockets/protobuf/third_party GameNetworkingSockets/protobuf/benchmarks
rm -r GameNetworkingSockets/GameNetworkingSockets/tests GameNetworkingSockets/protobuf/benchmarks
rm -r GameNetworkingSockets/libsodium-cmake/libsodium/test

# ozz-animation
rm -r ozz-animation/media ozz-animation/test ozz-animation/samples ozz-animation/howtos ozz-animation/extern/glfw ozz-animation/extern/gtest

# parallel-hashmap
rm -r parallel-hashmap/benchmark parallel-hashmap/cmake parallel-hashmap/css parallel-hashmap/examples parallel-hashmap/html parallel-hashmap/tests parallel-hashmap/index.html

# physfs
rm -r physfs/docs physfs/test

# tweeny
rm -r tweeny/doc tweeny/CHANGELOG.md

# glm
rm -r glm/util

# random
rm -r random/test

# backward
rm -r backward-cpp/doc backward-cpp/test backward-cpp/test_package

# boost
cd boost
rm -r doc/ more/ status/ tools/ boost.png bootstrap.sh bootstrap.bat index.htm index.html INSTALL Jamroot README.md rst.css boost.css boost-build.jam boostcpp.jam
cd ..

# date
rm -r date/ci date/compile_fail.sh date/README.md date/test date/test_fail.sh

# r8brain
rm -r r8brain-cmake/r8brain-free-src/DLL r8brain-cmake/r8brain-free-src/bench

# rlottie
rm -r RmlUi-freetype/rlottie/example RmlUi-freetype/rlottie/.gifs

# harfbuzz
rm -r RmlUi-freetype/harfbuzz/test RmlUi-freetype/harfbuzz/perf

# etl
rm -r etl/examples etl/images etl/support etl/temp etl/test etl/uml

# OpenXR
rm -r OpenXR-SDK/.azure-pipelines OpenXR-SDK/.github OpenXR-SDK/.reuse OpenXR-SDK/specification OpenXR-SDK/.appveyor.yml OpenXR-SDK/.editorconfig OpenXR-SDK/CHANGELOG.SDK.md

# sfizz
rm -r sfizz/benchmarks sfizz/tests sfizz/plugins