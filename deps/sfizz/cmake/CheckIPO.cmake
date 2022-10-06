# Added in CMake 3.9

if(CMAKE_VERSION VERSION_LESS 3.9)
    message(WARNING "\nIPO checks are only available on CMake 3.9 and later.")

    set(ENABLE_LTO OFF CACHE BOOL "" FORCE)

    function(SFIZZ_ENABLE_LTO_IF_NEEDED TARGET)
    endfunction()

    return()
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL armv7l)
    set(ENABLE_LTO OFF CACHE BOOL "" FORCE)
endif()

if(result AND ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "\nLTO enabled.")
else()
    if(${output})
        message(WARNING "\nIPO disabled: ${output}")
    else()
        message(WARNING "\nIPO was disabled or not in a Release build.")
    endif()
    set(ENABLE_LTO OFF CACHE BOOL "" FORCE)
endif()

function(SFIZZ_ENABLE_LTO_IF_NEEDED TARGET)
    if(${ENABLE_LTO})
        message(STATUS "Enabling LTO on ${TARGET}")
        set_property(TARGET ${TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()
endfunction()
