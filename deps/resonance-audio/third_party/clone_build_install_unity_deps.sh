#!/bin/bash
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

# Change working directory to script PATH
SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
cd "${SCRIPT_DIR}"

# Number of CPU cores/parallel compilation instances (for Darwin/Linux builds)
NUM_CORES=8

MSVC_GENERATOR="Visual Studio 14 2015"

declare -a EMBREE_CONFIG
EMBREE_CONFIG+=(-DEMBREE_ISPC_SUPPORT=OFF)
EMBREE_CONFIG+=(-DEMBREE_STATIC_LIB=ON)
EMBREE_CONFIG+=(-DEMBREE_TUTORIALS=OFF)
EMBREE_CONFIG+=(-DEMBREE_TASKING_SYSTEM=OFF)
EMBREE_CONFIG+=(-DEMBREE_BACKFACE_CULLING=ON)
EMBREE_CONFIG+=(-DEMBREE_MAX_ISA=SSE2)

declare -a BUILD_FLAGS
declare -a CONFIG_FLAGS

git_clone_if_not_exist () {
  TARGET_PATH=$1
  URL=$2
  BRANCH=$3
  PATCH=$4
  if [[ ! -d "${TARGET_PATH}" ]] ; then
    git clone -b "${BRANCH}" "${URL}" "${TARGET_PATH}"
    cd "$TARGET_PATH" && git checkout "${BRANCH}" && patch -p1 < ../patches/"${PATCH}" && cd ..
  fi
}

compile_embree_ogg_vorbis () {
  MAKE_GENERATOR=$1
  BUILD_PATH=$2
  INSTALL_PATH=$3

  CONFIG_WITH_GENERATOR=( "${CONFIG_FLAGS[@]}" )
  if [[ ! -z "${MAKE_GENERATOR}" ]]; then
    CONFIG_WITH_GENERATOR+=(-G"${MAKE_GENERATOR}")
  fi
  CONFIG_WITH_GENERATOR+=(-DCMAKE_INSTALL_PREFIX="${SCRIPT_DIR}/${INSTALL_PATH}/")

  # Create installation path.
  if [[ ! -d "$INSTALL_PATH" ]] ; then
    mkdir "${INSTALL_PATH}"
  fi

  cd "${SCRIPT_DIR}"
  EMBREE_CONFIG+=("${CONFIG_WITH_GENERATOR[@]}")
  cd embree && rm -fr "${BUILD_PATH}" && mkdir "${BUILD_PATH}" && cd "${BUILD_PATH}" &&\
    cmake "${EMBREE_CONFIG[@]}" .. &&\
    cmake --build . --config Release -- "${BUILD_FLAGS[@]}" && cd .. && cd ..

  # Build libOgg
  cd "${SCRIPT_DIR}"
  OGG_CONFIG=("${CONFIG_WITH_GENERATOR[@]}")
  OGG_CONFIG+=(-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true)
  cd ogg && rm -fr "${BUILD_PATH}" && mkdir "${BUILD_PATH}" && cd "${BUILD_PATH}" &&\
    cmake "${OGG_CONFIG[@]}" .. &&\
    cmake --build . --config Release --target install -- "${BUILD_FLAGS[@]}" && cd .. && cd ..

  # Build libVorbis
  cd "${SCRIPT_DIR}"
  VORBIS_CONFIG=("${CONFIG_WITH_GENERATOR[@]}")
  VORBIS_CONFIG+=(-DOGG_INCLUDE_DIRS="${SCRIPT_DIR}/${INSTALL_PATH}"/include/)
  VORBIS_CONFIG+=(-DOGG_LIBRARIES="${SCRIPT_DIR}/${INSTALL_PATH}"/)
  VORBIS_CONFIG+=(-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true)
  cd vorbis && rm -fr "${BUILD_PATH}" && mkdir "${BUILD_PATH}" && cd "${BUILD_PATH}" &&\
    cmake "${VORBIS_CONFIG[@]}" .. &&\
    cmake --build . --config Release --target install -- "${BUILD_FLAGS[@]}" && cd .. && cd ..
}

cd "${SCRIPT_DIR}"
git_clone_if_not_exist "embree" "https://github.com/embree/embree.git" "v2.16.5" "libembree.patch"
git_clone_if_not_exist "ogg" "https://github.com/xiph/ogg.git" "master" "libogg.patch"
git_clone_if_not_exist "vorbis" "https://github.com/xiph/vorbis" "master" "libvorbis.patch"
git_clone_if_not_exist "nativeaudioplugins" "https://github.com/Unity-Technologies/NativeAudioPlugins.git" "master"

case "$(uname -s)" in
  Darwin)
    CONFIG_FLAGS+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    BUILD_FLAGS+=(-j "${NUM_CORES}")
    DEFAULT_GENERATOR_FLAG=""
    compile_embree_ogg_vorbis "${DEFAULT_GENERATOR_FLAG}" "build" "install"
    ;;

  Linux)
    BUILD_FLAGS+=(-j "${NUM_CORES}")
    DEFAULT_GENERATOR_FLAG=""
    compile_embree_ogg_vorbis "${DEFAULT_GENERATOR_FLAG}" "build" "install"
    ;;

  CYGWIN*|MINGW*|MSYS*)
    CONFIG_FLAGS+=(-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON)
    EMBREE_CONFIG+=(-DEMBREE_STATIC_RUNTIME=ON)

    # Create 64bit builds
    WIN64_GENERATOR_FLAG="${MSVC_GENERATOR} Win64"
    compile_embree_ogg_vorbis "${WIN64_GENERATOR_FLAG}" "build64" "install64"

    # Create 32bit builds
    WIN32_GENERATOR_FLAG="${MSVC_GENERATOR}"
    compile_embree_ogg_vorbis "${WIN32_GENERATOR_FLAG}" "build32" "install32"
    ;;

  *)
    ;;
esac
