# Copyright 2018 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set(WWISE_SDK_DIR ${PROJECT_SOURCE_DIR}/third_party/WwiseIncludes/ CACHE PATH "Path to Wwise SDK")
set(WWISE_SDK_INCLUDE_DIR ${WWISE_SDK_DIR}/SDK/include)
set(WWISE_PLUGIN_DIR ${PROJECT_SOURCE_DIR}/platforms/wwise/)

set(RESONANCE_AUDIO_WWISE_PLUGIN_STATIC_SOURCES
        ${WWISE_PLUGIN_DIR}/wwise_common.h
        ${WWISE_PLUGIN_DIR}/wwise_fx_null_params.cc
        ${WWISE_PLUGIN_DIR}/wwise_fx_null_params.h
        ${WWISE_PLUGIN_DIR}/wwise_renderer_fx.cc
        ${WWISE_PLUGIN_DIR}/wwise_renderer_fx.h
        ${WWISE_PLUGIN_DIR}/wwise_room_effects_fx.cc
        ${WWISE_PLUGIN_DIR}/wwise_room_effects_fx.h
        ${WWISE_PLUGIN_DIR}/wwise_room_effects_fx_attachment_params.cc
        ${WWISE_PLUGIN_DIR}/wwise_room_effects_fx_attachment_params.h
        )

set(RESONANCE_AUDIO_WWISE_PLUGIN_SHARED_SOURCES
        ${WWISE_PLUGIN_DIR}/wwise_fx_shared.cc
        ${WWISE_PLUGIN_DIR}/wwise_fx_factory.h
        )
set(WWISE_INSTALL_DIR "${INSTALL_DIR}/wwise/")

if (BUILD_WWISE_SOUND_ENGINE_PLUGIN)
    add_library(ResonanceAudioFXShared SHARED
            ${RESONANCE_AUDIO_WWISE_PLUGIN_STATIC_SOURCES}
            ${RESONANCE_AUDIO_WWISE_PLUGIN_SHARED_SOURCES}
            $<TARGET_OBJECTS:ResonanceAudioObj>
            $<TARGET_OBJECTS:SadieHrtfsObj>
            $<TARGET_OBJECTS:PffftObj>)
    target_include_directories(ResonanceAudioFXShared PRIVATE ${WWISE_SDK_INCLUDE_DIR})
    target_include_directories(ResonanceAudioFXShared PRIVATE ${RA_SOURCE_DIR})
    set_property(TARGET ResonanceAudioFXShared PROPERTY POSITION_INDEPENDENT_CODE ON)

    add_library(ResonanceAudioFXStatic STATIC
            ${RESONANCE_AUDIO_WWISE_PLUGIN_STATIC_SOURCES}
            $<TARGET_OBJECTS:ResonanceAudioObj>
            $<TARGET_OBJECTS:SadieHrtfsObj>
            $<TARGET_OBJECTS:PffftObj>)
    target_include_directories(ResonanceAudioFXStatic PRIVATE ${WWISE_SDK_INCLUDE_DIR})
    target_include_directories(ResonanceAudioFXStatic PRIVATE ${RA_SOURCE_DIR})

    set(INSTALL_BIN_DIR "")
    if (MSVC)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(INSTALL_BIN_DIR "x64_vc140")
        else ()
            set(INSTALL_BIN_DIR "Win32_vc140")
        endif ()
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}/Debug")
        elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}/Release")
        else ()
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}/Profile")
        endif ()
        if (STATIC_MSVC_RUNTIME)
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}(StaticCRT)")
        endif ()
    elseif (ANDROID)
        if ("${ANDROID_ABI}" MATCHES "armeabi-v7a")
            set(INSTALL_BIN_DIR "android-9_armeabi-v7a")
        elseif ("${ANDROID_ABI}" MATCHES "x86")
            set(INSTALL_BIN_DIR "android-9_x86")
        elseif ("${ANDROID_ABI}" MATCHES "arm64-v8a")
            set(INSTALL_BIN_DIR "android-21_arm64-v8a")
        elseif ("${ANDROID_ABI}" MATCHES "x86_64")
            set(INSTALL_BIN_DIR "android-21_x86_64")
        endif ()
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}/Debug")
        elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}/Release")
        else ()
            set(INSTALL_BIN_DIR "${INSTALL_BIN_DIR}/Profile")
        endif ()
    elseif (APPLE)
        if (NOT IOS_DETECTED)
            if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(INSTALL_BIN_DIR "Mac/Debug")
            elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
                set(INSTALL_BIN_DIR "Mac/Release")
            else ()
                set(INSTALL_BIN_DIR "Mac/Profile")
            endif ()
        else ()
            if (IOS_PLATFORM STREQUAL SIMULATOR)
                set(IOS_PLATFORM_SUFFIX "-iphonesimulator/lib")
            elseif (IOS_PLATFORM STREQUAL OS)
                set(IOS_PLATFORM_SUFFIX "-iphoneos/lib")
            endif ()
            if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(INSTALL_BIN_DIR "iOS/Debug${IOS_PLATFORM_SUFFIX}")
            elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
                set(INSTALL_BIN_DIR "iOS/Release${IOS_PLATFORM_SUFFIX}")
            else ()
                set(INSTALL_BIN_DIR "iOS/Profile${IOS_PLATFORM_SUFFIX}")
            endif ()
        endif ()
    elseif (UNIX)
        set(INSTALL_BIN_DIR "Linux_x64")
    endif ()

    message(STATUS "Binary installation path: ${WWISE_INSTALL_DIR}/SDK/${INSTALL_BIN_DIR}/")
    install(TARGETS ResonanceAudioFXShared ResonanceAudioFXStatic
            LIBRARY DESTINATION "${WWISE_INSTALL_DIR}/SDK/${INSTALL_BIN_DIR}/bin/"
            ARCHIVE DESTINATION "${WWISE_INSTALL_DIR}/SDK/${INSTALL_BIN_DIR}/lib/"
            RUNTIME DESTINATION "${WWISE_INSTALL_DIR}/SDK/${INSTALL_BIN_DIR}/bin/")
    install(FILES "${PROJECT_SOURCE_DIR}/platforms/common/room_properties.h"
            DESTINATION "${WWISE_INSTALL_DIR}/SDK/include/ResonanceAudio/")
    install(FILES "${PROJECT_SOURCE_DIR}/platforms/wwise/wwise_fx_factory.h"
            DESTINATION "${WWISE_INSTALL_DIR}/SDK/include/AK/Plugin/" RENAME "ResonanceAudioFXFactory.h")
endif (BUILD_WWISE_SOUND_ENGINE_PLUGIN)

if (WIN32 AND BUILD_WWISE_AUTHORING_PLUGIN)
    set(RESONANCE_AUDIO_WWISE_AUTHORING_SOURCES
            ${WWISE_PLUGIN_DIR}/wwise_renderer_plugin.cc
            ${WWISE_PLUGIN_DIR}/wwise_renderer_plugin.h
            ${WWISE_PLUGIN_DIR}/wwise_room_effects_attachment_plugin.cc
            ${WWISE_PLUGIN_DIR}/wwise_room_effects_attachment_plugin.h
            ${WWISE_PLUGIN_DIR}/wwise_room_effects_plugin.cc
            ${WWISE_PLUGIN_DIR}/wwise_room_effects_plugin.h
            ${WWISE_PLUGIN_DIR}/wwise_win_app.cc
            ${WWISE_PLUGIN_DIR}/wwise_win_app.h
            )

    add_definitions(-D_AFXDLL)
    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
    add_definitions(-D_WINDLL)
    add_definitions(-D_USRDLL)
    add_definitions(-D_ATL_ALLOW_CHAR_UNSIGNED)


    FIND_PACKAGE(MFC)
    IF (NOT MFC_FOUND)
        message(FATAL_ERROR "MFC Could not be found during the MFC test")
    ENDIF ()
    set(CMAKE_MFC_FLAG 2)

    add_library(ResonanceAudio SHARED ${RESONANCE_AUDIO_WWISE_AUTHORING_SOURCES}
            ${RESONANCE_AUDIO_WWISE_PLUGIN_STATIC_SOURCES}
            $<TARGET_OBJECTS:ResonanceAudioObj>
            $<TARGET_OBJECTS:SadieHrtfsObj>
            $<TARGET_OBJECTS:PffftObj>)
    target_include_directories(ResonanceAudio PRIVATE ${WWISE_SDK_INCLUDE_DIR})
    target_include_directories(ResonanceAudio PRIVATE ${RA_SOURCE_DIR})
    target_sources(ResonanceAudio PUBLIC ${WWISE_PLUGIN_DIR}/wwise_win.def)
    set(INSTALL_BIN_DIR "")
    if ("${CMAKE_GENERATOR}" MATCHES "(Win64)")
        set(INSTALL_BIN_DIR "Authoring/x64/Release/bin/plugins")
    else ()
        set(INSTALL_BIN_DIR "Authoring/Win32/Release/bin/plugins")
    endif ()
    message(STATUS "Binary installation path: ${WWISE_INSTALL_DIR}/${INSTALL_BIN_DIR}/")
    install(TARGETS ResonanceAudio
            LIBRARY DESTINATION "${WWISE_INSTALL_DIR}/${INSTALL_BIN_DIR}/"
            RUNTIME DESTINATION "${WWISE_INSTALL_DIR}/${INSTALL_BIN_DIR}/"
            ARCHIVE DESTINATION "${WWISE_INSTALL_DIR}/${INSTALL_BIN_DIR}/")
    install(FILES "${PROJECT_SOURCE_DIR}/platforms/wwise/ResonanceAudio.xml"
            DESTINATION "${WWISE_INSTALL_DIR}/${INSTALL_BIN_DIR}")
endif (WIN32 AND BUILD_WWISE_AUTHORING_PLUGIN)
