# PhysX
rm -r physx/physx/documentation/ physx/physx/samples/ physx/externals/cg-linux/lib physx/externals/cg-linux/bin physx/externals/glew-linux/lib physx/externals/glew/lib physx/externals/opengl/lib64

# BGFX, BX
rm -r bgfx.cmake/bgfx/examples bgfx.cmake/bx/tools

# SDL
rm -r SDL2/test

# assimp
rm -r assimp/samples

# recast
rm -r recastnavigation/RecastDemo recastnavigation/Tests

# taskflow
rm -r taskflow/3rd-party taskflow/benchmarks taskflow/docs taskflow/doxygen taskflow/examples taskflow/image taskflow/legacy taskflow/sandbox taskflow/unittests

# concurrentqueue
rm -r concurrentqueue/benchmarks concurrentqueue/build concurrentqueue/c_api concurrentqueue/tests

# rmlui
rm -r RmlUI-freetype/RmlUi/Samples RmlUI-freetype/RmlUi/Tests

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
