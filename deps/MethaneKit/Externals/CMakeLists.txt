cmake_minimum_required(VERSION 3.11.0)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMRC"
    "${CMAKE_CURRENT_SOURCE_DIR}/Catch2/single_include/cmake"
)

add_subdirectory(CML/cml)
add_subdirectory(Catch2/single_include)
add_subdirectory(SPIRV)
add_subdirectory(DirectXCompiler)
add_subdirectory(D3DCompilerToDXC)
add_subdirectory(PerlinNoise)
add_subdirectory(CLI11)

add_subdirectory(IttApi)
set_target_properties(ittnotify
    PROPERTIES
    FOLDER Externals
)

# Nowide library
set(NOWIDE_BUILD_STATIC   ON  CACHE BOOL "Enable Nowide static build")
set(NOWIDE_BUILD_SHARED   OFF CACHE BOOL "Disable Nowide shared build")
set(NOWIDE_BUILD_INSTALL  OFF CACHE BOOL "Disable Nowide install build")
set(NOWIDE_BUILD_TESTS    OFF CACHE BOOL "Disable Nowide tests build")
set(NOWIDE_BUILD_DOC_HTML OFF CACHE BOOL "Disable Nowide HTML doc build")
set(NOWIDE_BUILD_DOC_CHM  OFF CACHE BOOL "Disable Nowide CHM doc build")
set(NOWIDE_BUILD_DOC_TEX  OFF CACHE BOOL "Disable Nowide TEX doc build")
add_subdirectory(Nowide)

# Image loader library: STB or OIIO
if (METHANE_USE_OPEN_IMAGE_IO)
    add_subdirectory(OpenImageIO)
else()
    add_subdirectory(STB)
endif()

# Platform dependent libraries
if (WIN32)
    add_subdirectory(DirectXLibraries/D3DX12)

    # DirectXTex library
    set(DIRECTXTEX_BUILD_TOOLS        OFF CACHE BOOL "Disable DirectXTex tools build")
    set(DIRECTXTEX_STANDALONE_ENABLED OFF CACHE BOOL "Disable DirectXTex standalone build")
    set(BUILD_DX11                    OFF CACHE BOOL "Disabled DirectXTex DirectX11 Runtime support")
    set(BUILD_DX12                    ON  CACHE BOOL "Enabled DirectXTex DirectX12 Runtime support")
    add_subdirectory(DirectXTex)
    set_target_properties(DirectXTex
        PROPERTIES
        FOLDER Externals
    )
endif()