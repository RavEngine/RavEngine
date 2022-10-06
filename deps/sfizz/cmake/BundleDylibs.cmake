# Dylib bundler for macOS
# Requires the external program "dylibbundler"

if(APPLE)
    find_program(DYLIBBUNDLER_PROGRAM "dylibbundler")
    if(NOT DYLIBBUNDLER_PROGRAM)
        message(WARNING "The installation helper \"dylibbundler\" is not available.")
    endif()
endif()

function(bundle_dylibs NAME PATH)
    if(NOT APPLE OR NOT DYLIBBUNDLER_PROGRAM)
        return()
    endif()

    set(_relative_libdir "../Frameworks")

    get_filename_component(_dir "${PATH}" DIRECTORY)
    set(_dir "${_dir}/${_relative_libdir}")

    set(_script "${CMAKE_CURRENT_BINARY_DIR}/_bundle-dylibs.${NAME}.cmake")

    file(WRITE "${_script}"
"execute_process(COMMAND \"${DYLIBBUNDLER_PROGRAM}\"
    \"-cd\" \"-of\" \"-b\"
    \"-x\" \"\$ENV{DESTDIR}${PATH}\"
    \"-d\" \"\$ENV{DESTDIR}${_dir}\"
    \"-p\" \"@loader_path/${_relative_libdir}/\")
")

    install(SCRIPT "${_script}" ${ARGN})
endfunction()
