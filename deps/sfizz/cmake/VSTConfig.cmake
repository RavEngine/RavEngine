set(VSTPLUGIN_NAME            "sfizz")
set(VSTPLUGIN_VENDOR          "Paul Ferrand")
set(VSTPLUGIN_URL             "http://sfztools.github.io/sfizz")
set(VSTPLUGIN_EMAIL           "paul@ferrand.cc")

if(APPLE)
    set(VSTPLUGIN_INSTALL_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3" CACHE STRING
    "Install destination for VST bundle [default: $ENV{HOME}/Library/Audio/Plug-Ins/VST3]")
    set(AUPLUGIN_INSTALL_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components" CACHE STRING
    "Install destination for AudioUnit bundle [default: $ENV{HOME}/Library/Audio/Plug-Ins/Components]")
elseif(MSVC)
    set(VSTPLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/vst3" CACHE STRING
    "Install destination for VST bundle [default: ${CMAKE_INSTALL_PREFIX}/vst3]")
else()
    set(VSTPLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/vst3" CACHE STRING
    "Install destination for VST bundle [default: ${CMAKE_INSTALL_PREFIX}/lib/vst3]")
endif()

if(NOT VST3_SYSTEM_PROCESSOR)
    set(VST3_SYSTEM_PROCESSOR "${SFIZZ_SYSTEM_PROCESSOR}")
endif()

message(STATUS "The system architecture is: ${VST3_SYSTEM_PROCESSOR}")

# --- VST3 Bundle architecture ---
if(NOT VST3_PACKAGE_ARCHITECTURE)
    if(APPLE)
        # VST3 packages are universal on Apple, architecture string not needed
    else()
        if(VST3_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|x64|X64)$")
            set(VST3_PACKAGE_ARCHITECTURE "x86_64")
        elseif(VST3_SYSTEM_PROCESSOR MATCHES "^(i.86|x86|X86)$")
            if(WIN32)
                set(VST3_PACKAGE_ARCHITECTURE "x86")
            else()
                set(VST3_PACKAGE_ARCHITECTURE "i386")
            endif()
        elseif(VST3_SYSTEM_PROCESSOR MATCHES "^(armv[3-8][a-z]*)$")
            set(VST3_PACKAGE_ARCHITECTURE "${VST3_SYSTEM_PROCESSOR}")
        elseif(VST3_SYSTEM_PROCESSOR MATCHES "^(aarch64)$")
            set(VST3_PACKAGE_ARCHITECTURE "aarch64")
        else()
            message(FATAL_ERROR "We don't know this architecture for VST3: ${VST3_SYSTEM_PROCESSOR}.")
        endif()
    endif()
endif()

message(STATUS "The VST3 architecture is deduced as: ${VST3_PACKAGE_ARCHITECTURE}")
