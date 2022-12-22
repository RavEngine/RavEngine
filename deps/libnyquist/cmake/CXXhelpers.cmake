
function(_add_define definition)
    list(APPEND _NQR_CXX_DEFINITIONS "-D${definition}")
    set(_NQR_CXX_DEFINITIONS ${_NQR_CXX_DEFINITIONS} PARENT_SCOPE)
endfunction()

function(_disable_warning flag)
    if(MSVC)
        list(APPEND _NQR_CXX_WARNING_FLAGS "/wd${flag}")
    else()
        list(APPEND _NQR_CXX_WARNING_FLAGS "-Wno-${flag}")
    endif()
    set(_NQR_CXX_WARNING_FLAGS ${_NQR_CXX_WARNING_FLAGS} PARENT_SCOPE)
endfunction()

function(_set_compile_options proj)
    if (WIN32)
        target_compile_options(${proj} PRIVATE /arch:AVX /Zi )
    endif()
endfunction()

function(set_cxx_version proj)
    target_compile_features(${proj} INTERFACE cxx_std_14)
endfunction()
