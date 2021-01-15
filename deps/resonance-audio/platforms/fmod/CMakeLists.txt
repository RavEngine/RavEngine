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

if (WIN32)
    set(FMOD_SDK_DIR "C:/Program\ Files\ (x86)/FMOD\ SoundSystem/FMOD\ Studio\ API\ Windows" CACHE PATH "Path to FMOD SDK")
elseif (APPLE)
    set(FMOD_SDK_DIR "${PROJECT_SOURCE_DIR}/third_party/FMOD\ Programmers\ API" CACHE PATH "Path to FMOD SDK")
else ()
    set(FMOD_SDK_DIR "${PROJECT_SOURCE_DIR}/third_party/fmod" CACHE PATH "Path to FMOD SDK")
endif ()

set(RA_FMOD_SOURCE_DIR ${PROJECT_SOURCE_DIR}/platforms/fmod)
set(RESONANCE_AUDIO_FMOD_PLUGIN_SOURCES
        ${RA_FMOD_SOURCE_DIR}/fmod.cc
        ${RA_FMOD_SOURCE_DIR}/fmod.h
        )

add_library(resonanceaudio SHARED ${RESONANCE_AUDIO_FMOD_PLUGIN_SOURCES} $<TARGET_OBJECTS:ResonanceAudioObj>
        $<TARGET_OBJECTS:SadieHrtfsObj>
        $<TARGET_OBJECTS:PffftObj>)
target_include_directories(resonanceaudio PRIVATE ${FMOD_SDK_DIR})
target_include_directories(resonanceaudio PRIVATE ${RA_SOURCE_DIR})
if (MSVC)
    target_sources(resonanceaudio PUBLIC ${RA_FMOD_SOURCE_DIR}/fmod_win.def)
endif (MSVC)

set(FMOD_INSTALL_DIR "${INSTALL_DIR}/fmod/")
set(INSTALL_BIN_DIR "")
if (MSVC)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(INSTALL_BIN_DIR "Plugins/Win/x64")
    else ()
        set(INSTALL_BIN_DIR "Plugins/Win/x86")
    endif ()
elseif (ANDROID)
    if ("${ANDROID_ABI}" MATCHES "armeabi-v7a")
        set(INSTALL_BIN_DIR "Plugins/Android/armeabi-v7a")
    elseif ("${ANDROID_ABI}" MATCHES "x86")
        set(INSTALL_BIN_DIR "Plugins/Android/x86")
    elseif ("${ANDROID_ABI}" MATCHES "arm64-v8a")
        set(INSTALL_BIN_DIR "Plugins/Android/arm64-v8a")
    elseif ("${ANDROID_ABI}" MATCHES "x86_64")
        set(INSTALL_BIN_DIR "Plugins/Android/x86_64")
    endif ()
elseif (APPLE)
    if (IOS_DETECTED)
        set(INSTALL_BIN_DIR "Plugins/iOS")
    else ()
        set(INSTALL_BIN_DIR "Plugins/Mac")
        set_target_properties(resonanceaudio PROPERTIES
                PREFIX ""
                )
    endif ()
elseif (UNIX)
    set(INSTALL_BIN_DIR "Plugins/Linux")
endif ()

install(TARGETS resonanceaudio
        LIBRARY DESTINATION "${FMOD_INSTALL_DIR}/${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${FMOD_INSTALL_DIR}/${INSTALL_BIN_DIR}"
        RUNTIME DESTINATION "${FMOD_INSTALL_DIR}/${INSTALL_BIN_DIR}")
install(DIRECTORY ${RA_FMOD_SOURCE_DIR}/UnityIntegration DESTINATION ${FMOD_INSTALL_DIR})
install(FILES "${PROJECT_SOURCE_DIR}/platforms/common/room_properties.h"
        DESTINATION "${FMOD_INSTALL_DIR}/Plugins/include/")
