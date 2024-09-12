include(FetchContent)

option(AGILITY_SDK_MAJOR_VERSION "Agility SDK major version")
option(AGILITY_SDK_MINOR_VERSION "Agility SDK minor version")
option(AGILITY_SDK_OUTPUT_DIR "Agility SDK output directory")

if (NOT AGILITY_SDK_MAJOR_VERSION)
  set(AGILITY_SDK_MAJOR_VERSION "1")
endif()
if (NOT AGILITY_SDK_MINOR_VERSION)
  set(AGILITY_SDK_MINOR_VERSION "614")
endif()
if (NOT AGILITY_SDK_OUTPUT_DIR)
  message(FATAL_ERROR "AGILITY_SDK_OUTPUT_DIR must be set")
endif()

set(AGILITY_SDK_VERSION "${AGILITY_SDK_MAJOR_VERSION}.${AGILITY_SDK_MINOR_VERSION}.0")

set(_AGILITY_SDK_BASE_NUGET_URL "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12")
set(_AGILITY_SDK_NUGET_URL "${_AGILITY_SDK_BASE_NUGET_URL}/${AGILITY_SDK_VERSION}")

FetchContent_Declare(
  agilitysdk
  URL "${_AGILITY_SDK_NUGET_URL}"
)
FetchContent_MakeAvailable(agilitysdk)

if (WIN32)
  if (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
    set(_AGILITY_SDK_ARCH "x64")
  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86")
    set(_AGILITY_SDK_ARCH "win32")
  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
    set(_AGILITY_SDK_ARCH "arm64")
  endif()
else()
  message(FATAL_ERROR "Agility SDK is only supported on Windows")
endif()

set(_AGILITY_SDK_BINARY_DIR "${agilitysdk_SOURCE_DIR}/build/native/bin/${_AGILITY_SDK_ARCH}")
set(_AGILITY_SDK_INCLUDE_DIR "${agilitysdk_SOURCE_DIR}/build/native/include")

add_library(AgilitySDK INTERFACE)
target_include_directories(AgilitySDK INTERFACE "${_AGILITY_SDK_INCLUDE_DIR}")

add_custom_target(_AgilitySDKInternal ALL)
add_custom_command(TARGET
  _AgilitySDKInternal POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${_AGILITY_SDK_BINARY_DIR}" "${AGILITY_SDK_OUTPUT_DIR}"
)
message(${AGILITY_SDK_OUTPUT_DIR})