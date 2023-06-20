# ~~~
# Copyright (c) 2014-2023 The Khronos Group Inc.
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
# Copyright (c) 2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

add_library(loader_specific_options INTERFACE)
target_link_libraries(loader_specific_options INTERFACE loader_common_options Vulkan::Headers)
target_include_directories(loader_specific_options INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/generated ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)

    if(ENABLE_WIN10_ONECORE)
        # Note: When linking your app or driver to OneCore.lib, be sure to remove any links to non-umbrella libs (such as
        # kernel32.lib).
        set(CMAKE_CXX_STANDARD_LIBRARIES " ") # space is intentional
        set(CMAKE_C_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES})
    endif()

    target_compile_options(loader_specific_options INTERFACE -D_CRT_SECURE_NO_WARNINGS)
    # ~~~
    # Build dev_ext_trampoline.c and unknown_ext_chain.c with /O2 to allow tail-call optimization.
    # Setup two CMake targets (loader-norm and loader-opt) for the different compilation flags.
    # ~~~
    set(MODIFIED_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

    string(REPLACE "/Od" "/O2" MODIFIED_C_FLAGS_DEBUG ${MODIFIED_C_FLAGS_DEBUG})
    string(REPLACE "/Ob0" "/Ob2" MODIFIED_C_FLAGS_DEBUG ${MODIFIED_C_FLAGS_DEBUG})
    string(REGEX REPLACE "/RTC." "" MODIFIED_C_FLAGS_DEBUG ${MODIFIED_C_FLAGS_DEBUG})  #remove run-time error checks

    separate_arguments(MODIFIED_C_FLAGS_DEBUG WINDOWS_COMMAND ${MODIFIED_C_FLAGS_DEBUG})

    # ~~~
    # Only generate the loader.rc file with CMake if BUILD_DLL_VERSIONINFO was set.
    # This feature is for the Vulkan Runtime build
    # Otherwise rely on the checked in loader.rc from the python script
    # ~~~
    if (NOT "$CACHE{BUILD_DLL_VERSIONINFO}" STREQUAL "")
        string(TIMESTAMP CURRENT_YEAR "%Y")
        set(LOADER_CUR_COPYRIGHT_YEAR "${CURRENT_YEAR}")
        set(LOADER_RC_VERSION "$CACHE{BUILD_DLL_VERSIONINFO}")
        set(LOADER_VER_FILE_VERSION_STR "\"${LOADER_RC_VERSION}\"")
        set(LOADER_VER_FILE_DESCRIPTION_STR "\"Vulkan Loader\"")

        # RC file wants the value of FILEVERSION to separated by commas
        string(REPLACE "." ", " LOADER_VER_FILE_VERSION "${LOADER_RC_VERSION}")

        # Configure the file to include the versioning info
        # Place it in the build directory - the GN build will use the checked in file
        configure_file(loader.rc.in ${CMAKE_CURRENT_BINARY_DIR}/loader.rc)
    endif()
else()
    # Used to make alloca() and secure_getenv() available
    target_compile_definitions(loader_specific_options INTERFACE _GNU_SOURCE)
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        target_compile_definitions(loader_specific_options INTERFACE __BSD_VISIBLE=1)
    endif()
    check_include_file("alloca.h" HAVE_ALLOCA_H)
    if(HAVE_ALLOCA_H)
        target_compile_definitions(loader_specific_options INTERFACE HAVE_ALLOCA_H)
    endif()
endif()

set(NORMAL_LOADER_SRCS
    allocation.c
    allocation.h
    cJSON.c
    cJSON.h
    debug_utils.c
    debug_utils.h
    extension_manual.c
    extension_manual.h
    loader_environment.c
    loader_environment.h
    gpa_helper.c
    gpa_helper.h
    loader.c
    loader.h
    log.c
    log.h
    settings.c
    settings.h
    terminator.c
    trampoline.c
    unknown_function_handling.c
    unknown_function_handling.h
    wsi.c
    wsi.h
    )

if(WIN32)
    list(APPEND NORMAL_LOADER_SRCS loader_windows.c dirent_on_windows.c)
elseif(UNIX AND NOT APPLE) # i.e.: Linux
    list(APPEND NORMAL_LOADER_SRCS loader_linux.c)
    target_compile_definitions(loader_specific_options INTERFACE LOADER_ENABLE_LINUX_SORT)
endif()

set(OPT_LOADER_SRCS dev_ext_trampoline.c phys_dev_ext.c)

# Check for assembler support
set(ASM_FAILURE_MSG "The build will fall back on building with C code\n")
set(ASM_FAILURE_MSG "${ASM_FAILURE_MSG}Note that this may be unsafe, as the C code requires tail-call optimizations to remove")
set(ASM_FAILURE_MSG "${ASM_FAILURE_MSG} the stack frame for certain calls. If the compiler does not do this, then unknown device")
set(ASM_FAILURE_MSG "${ASM_FAILURE_MSG} extensions will suffer from a corrupted stack.")
if(WIN32)
    if(MINGW)
        find_program(JWASM_FOUND jwasm)
        if (JWASM_FOUND)
            set(CMAKE_ASM_MASM_COMPILER ${JWASM_FOUND})
            execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE COMPILER_VERSION_OUTPUT)
            if (COMPILER_VERSION_OUTPUT)
                if (COMPILER_VERSION_OUTPUT MATCHES "x86_64")
                    set(JWASM_FLAGS -win64)
                else()
                    # jwasm requires setting the cpu to at least 386 for setting a flat model
                    set(JWASM_FLAGS -3 -coff)
                endif()
            endif()
        endif()
    endif()
    option(USE_MASM "Use MASM" ON)
    if (USE_MASM)
      enable_language(ASM_MASM)
    endif ()
    if(CMAKE_ASM_MASM_COMPILER_WORKS OR JWASM_FOUND)
        if(MINGW)
            set(CMAKE_ASM_MASM_FLAGS ${CMAKE_ASM_MASM_FLAGS} ${JWASM_FLAGS})
        elseif(NOT CMAKE_CL_64 AND NOT JWASM_FOUND)
            set(CMAKE_ASM_MASM_FLAGS ${CMAKE_ASM_MASM_FLAGS} /safeseh)
        endif()

        add_executable(asm_offset asm_offset.c)
        target_link_libraries(asm_offset PRIVATE loader_specific_options)
        # If not cross compiling, run asm_offset to generage gen_defines.asm
        if (NOT CMAKE_CROSSCOMPILING)
            add_custom_command(OUTPUT gen_defines.asm DEPENDS asm_offset COMMAND asm_offset MASM)
        else()
            # Forces compiler to write the intermediate asm file, needed so that we can get sizeof/offset of info out of it.
            target_compile_options(asm_offset PRIVATE "/Fa$<TARGET_FILE_DIR:asm_offset>/asm_offset.asm" /FA)
            # Force off optimization so that the output assembly includes all the necessary info - optimizer would get rid of it otherwise.
            target_compile_options(asm_offset PRIVATE /Od)

            find_package(Python3 REQUIRED QUIET)
            # Run parse_asm_values.py on asm_offset's assembly file to generate the gen_defines.asm, which the asm code depends on
            add_custom_command(TARGET asm_offset POST_BUILD
                COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/parse_asm_values.py "${CMAKE_CURRENT_BINARY_DIR}/gen_defines.asm"
                    "$<TARGET_FILE_DIR:asm_offset>/asm_offset.asm" "MASM" "${CMAKE_CXX_COMPILER_ID}" "${CMAKE_SYSTEM_PROCESSOR}"
                BYPRODUCTS gen_defines.asm
            )
        endif()
        add_custom_target(loader_asm_gen_files DEPENDS gen_defines.asm)
        set_target_properties(loader_asm_gen_files PROPERTIES FOLDER ${LOADER_HELPER_FOLDER})

        add_library(loader-unknown-chain OBJECT unknown_ext_chain_masm.asm)
        target_link_libraries(loader-unknown-chain Vulkan::Headers)
        target_include_directories(loader-unknown-chain PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(loader-unknown-chain loader_asm_gen_files)
    else()
        set(USE_ASSEMBLY_FALLBACK ON)
        message(WARNING "Could not find working MASM assembler\n${ASM_FAILURE_MSG}")
    endif()
elseif(UNIX) # i.e.: Linux & Apple
    option(USE_GAS "Use GAS" ON)
    if(USE_GAS)
        enable_language(ASM)

        set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
        set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

        if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
            try_compile(ASSEMBLER_WORKS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/asm_test_aarch64.S)
            if(ASSEMBLER_WORKS)
                set(OPT_LOADER_SRCS ${OPT_LOADER_SRCS} unknown_ext_chain_gas_aarch64.S)
            endif()
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "^i.86$")
            check_include_file("cet.h" HAVE_CET_H)
            if(HAVE_CET_H)
                target_compile_definitions(loader_specific_options INTERFACE HAVE_CET_H)
            endif()

            try_compile(ASSEMBLER_WORKS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/asm_test_x86.S)
            if(ASSEMBLER_WORKS)
                set(OPT_LOADER_SRCS ${OPT_LOADER_SRCS} unknown_ext_chain_gas_x86.S)
            endif()
        endif()
    endif()

    # When compiling for x86 on x64, we can't use CMAKE_SYSTEM_PROCESSOR to determine which architecture to use,
    # Instead, check the size of void* and if its 4, set ASM_OFFSET_SYSTEM_PROCESSOR to x86
    # Note - there is no 32 bit arm assembly code, so this only applies to x86 currently.
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(ASM_OFFSET_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}) # x86_64 or aarch64
    else()
        set(ASM_OFFSET_SYSTEM_PROCESSOR "x86")
    endif()

    if(ASSEMBLER_WORKS)
        add_executable(asm_offset asm_offset.c)
        target_link_libraries(asm_offset loader_specific_options)
        # If not cross compiling, run asm_offset to generage gen_defines.asm
        if (NOT CMAKE_CROSSCOMPILING)
            add_custom_command(OUTPUT gen_defines.asm DEPENDS asm_offset COMMAND asm_offset GAS)
        else()
            # Forces compiler to write the intermediate asm file, needed so that we can get sizeof/offset of info out of it.
            target_compile_options(asm_offset PRIVATE -save-temps=obj)
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                set(ASM_OFFSET_INTERMEDIATE_LOCATION "$<TARGET_FILE_DIR:asm_offset>/CMakeFiles/asm_offset.dir/asm_offset.c.s")
            elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                set(ASM_OFFSET_INTERMEDIATE_LOCATION "$<TARGET_FILE_DIR:asm_offset>/CMakeFiles/asm_offset.dir/asm_offset.s")
            endif()

            find_package(Python3 REQUIRED QUIET)
            # Run parse_asm_values.py on asm_offset's assembly file to generate the gen_defines.asm, which the asm code depends on
            add_custom_command(TARGET asm_offset POST_BUILD
                COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/parse_asm_values.py "$<TARGET_FILE_DIR:asm_offset>/gen_defines.asm"
                    "${ASM_OFFSET_INTERMEDIATE_LOCATION}" "GAS" "${CMAKE_CXX_COMPILER_ID}" "${ASM_OFFSET_SYSTEM_PROCESSOR}"
                BYPRODUCTS gen_defines.asm
            )
        endif()
        add_custom_target(loader_asm_gen_files DEPENDS gen_defines.asm)

        if (APPLE)
            set(MODIFY_UNKNOWN_FUNCTION_DECLS ON)
        endif()
    else()
        set(USE_ASSEMBLY_FALLBACK ON)
        if(USE_GAS)
            message(WARNING "Could not find working ${ASM_OFFSET_SYSTEM_PROCESSOR} GAS assembler\n${ASM_FAILURE_MSG}")
        else()
            message(WARNING "Assembly sources have been disabled\n${ASM_FAILURE_MSG}")
        endif()
    endif()
endif()

if(USE_ASSEMBLY_FALLBACK)
    add_custom_target(loader_asm_gen_files)
    if (MSVC)
        add_library(loader-unknown-chain OBJECT unknown_ext_chain.c)
        target_link_libraries(loader-unknown-chain loader_specific_options)
        set_target_properties(loader-unknown-chain PROPERTIES CMAKE_C_FLAGS_DEBUG "${MODIFIED_C_FLAGS_DEBUG}")
    else()
        set(OPT_LOADER_SRCS ${OPT_LOADER_SRCS} unknown_ext_chain.c)
        set_source_files_properties(${OPT_LOADER_SRCS} PROPERTIES COMPILE_FLAGS -O)
    endif()
endif()

if(WIN32)
    add_library(loader-opt STATIC ${OPT_LOADER_SRCS})
    target_link_libraries(loader-opt PUBLIC loader_specific_options)
    add_dependencies(loader-opt loader_asm_gen_files)
    set_target_properties(loader-opt PROPERTIES CMAKE_C_FLAGS_DEBUG "${MODIFIED_C_FLAGS_DEBUG}")

    # If BUILD_DLL_VERSIONINFO was set, use the loader.rc in the build dir, otherwise use the checked in file
    set(RC_FILE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/loader.rc)
    if (NOT "$CACHE{BUILD_DLL_VERSIONINFO}" STREQUAL "")
        set(RC_FILE_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/loader.rc)
    endif()

    if (MSVC)
        set(LOADER_UNKNOWN_CHAIN_LIBRARY $<TARGET_OBJECTS:loader-unknown-chain>)
    else()
        set(LOADER_UNKNOWN_CHAIN_LIBRARY "")
    endif()

    add_library(vulkan
                SHARED
                ${NORMAL_LOADER_SRCS}
                ${LOADER_UNKNOWN_CHAIN_LIBRARY}
                ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-1.def
                ${RC_FILE_LOCATION})

    target_link_libraries(vulkan PRIVATE loader_specific_options loader-opt)

    # when adding the suffix the import and runtime library names must be consistent
    # mingw: libvulkan-1.dll.a / vulkan-1.dll
    # msvc: vulkan-1.lib / vulkan-1.dll
    set_target_properties(vulkan
                          PROPERTIES
                          OUTPUT_NAME vulkan-1)
    if(MINGW)
        # generate the same DLL with mingw
        set_target_properties(vulkan
                              PROPERTIES
                              PREFIX "")
    endif()

    if(MSVC AND ENABLE_WIN10_ONECORE)
        target_link_libraries(vulkan PRIVATE OneCoreUAP.lib LIBCMT.LIB LIBCMTD.LIB LIBVCRUNTIME.LIB LIBUCRT.LIB)
        set_target_properties(vulkan PROPERTIES LINK_FLAGS "/NODEFAULTLIB")
    else()
       target_link_libraries(vulkan PRIVATE cfgmgr32)
    endif()

    add_dependencies(vulkan loader_asm_gen_files)

else()
    if(APPLE AND BUILD_STATIC_LOADER)
        add_library(vulkan STATIC ${NORMAL_LOADER_SRCS} ${OPT_LOADER_SRCS})
        target_compile_definitions(vulkan PRIVATE BUILD_STATIC_LOADER)
    else()
        add_library(vulkan SHARED ${NORMAL_LOADER_SRCS} ${OPT_LOADER_SRCS})
    endif()
    add_dependencies(vulkan loader_asm_gen_files)
    set_target_properties(vulkan
                          PROPERTIES SOVERSION "1"
                          VERSION ${VULKAN_LOADER_VERSION})
    target_link_libraries(vulkan PRIVATE ${CMAKE_DL_LIBS} m)
    if (NOT ANDROID)
        target_link_libraries(vulkan PRIVATE Threads::Threads)
    endif()

    if (LOADER_ENABLE_ADDRESS_SANITIZER)
        target_compile_options(vulkan PUBLIC -fsanitize=address)
        target_link_options(vulkan PUBLIC -fsanitize=address)
    endif()
    if (LOADER_ENABLE_THREAD_SANITIZER)
        target_compile_options(vulkan PUBLIC -fsanitize=thread)
        target_link_options(vulkan PUBLIC -fsanitize=thread)
    endif()

    if(APPLE)
        find_library(COREFOUNDATION_LIBRARY NAMES CoreFoundation)
        target_link_libraries(vulkan PRIVATE "-framework CoreFoundation")

        # Build vulkan.framework
        # Use GLOB_RECURSE to find all the header files and populate the vulkan.framework headers with them
        # Use CONFIGURE_DEPENDS to ensure that if the header files are updated, this list is also updated
        get_target_property(VulkanHeaders_INCLUDE_DIRS Vulkan::Headers INTERFACE_INCLUDE_DIRECTORIES)
        file(GLOB_RECURSE CONFIGURE_DEPENDS FRAMEWORK_HEADERS ${VulkanHeaders_INCLUDE_DIRS})
        if(BUILD_STATIC_LOADER)
            add_library(vulkan-framework STATIC ${NORMAL_LOADER_SRCS} ${OPT_LOADER_SRCS} ${FRAMEWORK_HEADERS})
        else()
            add_library(vulkan-framework SHARED ${NORMAL_LOADER_SRCS} ${OPT_LOADER_SRCS} ${FRAMEWORK_HEADERS})
        endif()
        add_dependencies(vulkan-framework loader_asm_gen_files)
        target_link_libraries(vulkan-framework ${CMAKE_DL_LIBS} Threads::Threads -lm "-framework CoreFoundation")
        target_link_libraries(vulkan-framework loader_specific_options)

        if (MODIFY_UNKNOWN_FUNCTION_DECLS)
            # Modifies the names of functions as they appearin the assembly code so that the
            # unknown function handling will work
            target_compile_definitions(vulkan PRIVATE MODIFY_UNKNOWN_FUNCTION_DECLS)
            target_compile_definitions(vulkan-framework PRIVATE MODIFY_UNKNOWN_FUNCTION_DECLS)
        endif()

        # The FRAMEWORK_VERSION needs to be "A" here so that Xcode code-signing works when a user adds their framework to an Xcode
        # project and does "Sign on Copy". It would have been nicer to use "1" to denote Vulkan 1. Although Apple docs say that a
        # framework version does not have to be "A", this part of the Apple toolchain expects it.
        # https://forums.developer.apple.com/thread/65963

# cmake-format: off
        set_target_properties(vulkan-framework PROPERTIES
            OUTPUT_NAME vulkan
            FRAMEWORK TRUE
            FRAMEWORK_VERSION A
            VERSION "${VULKAN_LOADER_VERSION}" # "current generated version"
            SOVERSION "1.0.0"                  # "compatibility version"
            MACOSX_FRAMEWORK_IDENTIFIER com.lunarg.vulkanFramework
            PUBLIC_HEADER "${FRAMEWORK_HEADERS}"
        )
        install(TARGETS vulkan-framework
            PUBLIC_HEADER DESTINATION vulkan
            FRAMEWORK DESTINATION loader
        )
# cmake-format: on
    endif()
endif()

option(LOADER_USE_UNSAFE_FILE_SEARCH "Allows the loader to search in unsafe locations")
if (LOADER_USE_UNSAFE_FILE_SEARCH)
    target_compile_definitions(vulkan PRIVATE LOADER_USE_UNSAFE_FILE_SEARCH)
endif()

option(LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING "Causes the loader to not unload dynamic libraries.")
if (LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING)
    target_compile_definitions(vulkan PRIVATE LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING)
endif()

# common attributes of the vulkan library
target_link_libraries(vulkan PRIVATE loader_specific_options)

set_target_properties(vulkan PROPERTIES ${LOADER_STANDARD_C_PROPERTIES})
if (TARGET asm_offset)
    set_target_properties(asm_offset PROPERTIES ${LOADER_STANDARD_C_PROPERTIES})
endif()

# Generate pkg-config file.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    set(PRIVATE_LIBS "")
    if (APPLE AND BUILD_STATIC_LOADER)
        # Libs.private should only be present when building a static loader
        foreach(LIB ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES})
            list(APPEND PRIVATE_LIBS "-l${LIB}")
        endforeach()
        list(REMOVE_DUPLICATES PRIVATE_LIBS)
        set(PRIVATE_LIBS "Libs.private: ${PRIVATE_LIBS}")
    endif()
    if(WIN32)
        if(MINGW)
            set(VULKAN_LIB_SUFFIX "-1.dll")
        else()
            set(VULKAN_LIB_SUFFIX "-1")
        endif()
        # Set libdir path as in cmake's FindVulkan.cmake
        # https://github.com/KhronosGroup/Vulkan-Loader/issues/668
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            string(REPLACE "lib" "Lib" CMAKE_INSTALL_FULL_LIBDIR_PC ${CMAKE_INSTALL_FULL_LIBDIR})
        else()
            string(REPLACE "lib" "Lib32" CMAKE_INSTALL_FULL_LIBDIR_PC ${CMAKE_INSTALL_FULL_LIBDIR})
        endif()
    else()
        set(CMAKE_INSTALL_FULL_LIBDIR_PC ${CMAKE_INSTALL_FULL_LIBDIR})
    endif ()
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "")
        set(CMAKE_INSTALL_REL_LIBDIR_PC ${CMAKE_INSTALL_FULL_LIBDIR_PC})
        set(CMAKE_INSTALL_REL_INCLUDEDIR_PC ${CMAKE_INSTALL_FULL_INCLUDEDIR})
    else()
        file(RELATIVE_PATH CMAKE_INSTALL_REL_LIBDIR_PC ${CMAKE_INSTALL_PREFIX} ${CMAKE_INSTALL_FULL_LIBDIR_PC})
        file(RELATIVE_PATH CMAKE_INSTALL_REL_INCLUDEDIR_PC ${CMAKE_INSTALL_PREFIX} ${CMAKE_INSTALL_FULL_INCLUDEDIR})
    endif()
    set(VULKAN_LOADER_PKG_CONFIG_NAME "Vulkan-Loader")
    configure_file("vulkan.pc.in" "vulkan.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vulkan.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

target_link_libraries(vulkan PRIVATE Vulkan::Headers)
add_library(Vulkan::Vulkan ALIAS vulkan)

install(TARGETS vulkan
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
