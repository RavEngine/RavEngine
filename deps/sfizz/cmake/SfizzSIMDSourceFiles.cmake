macro(sfizz_add_simd_sources SOURCES_VAR PREFIX)
    # It needs a macro, otherwise the source properties cannot take effect.

    list(APPEND ${SOURCES_VAR}
        ${PREFIX}/sfizz/SIMDHelpers.cpp
        ${PREFIX}/sfizz/simd/HelpersNEON.cpp
        ${PREFIX}/sfizz/simd/HelpersSSE.cpp
        ${PREFIX}/sfizz/simd/HelpersAVX.cpp)

    # For CPU-dispatched X86 sources
    # Always build them for all X86 targets.
    if(SFIZZ_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|x64|X64|i.86|x86|X86)$")
        # on GCC, it requires to set ISA support flags on individual files
        # to be able to use the intrinsics
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            set_source_files_properties(
                ${PREFIX}/sfizz/effects/impl/ResonantStringAVX.cpp
                ${PREFIX}/sfizz/effects/impl/ResonantArrayAVX.cpp
                ${PREFIX}/sfizz/simd/HelpersAVX.cpp
                PROPERTIES COMPILE_FLAGS "-mavx")
        endif()
    endif()
endmacro()
