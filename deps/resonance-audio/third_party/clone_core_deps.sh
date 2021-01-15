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

# Change working directory to script folder
SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

git_clone_if_not_exist () {
  FOLDER=$1
  URL=$2
  BRANCH=$3
  if [[ ! -d "$FOLDER" ]] ; then
    git clone -b "${BRANCH}" "$URL" "$FOLDER"
  fi
}

cd "${SCRIPT_DIR}"
# Clone Eigen
git_clone_if_not_exist "eigen" "https://gitlab.com/libeigen/eigen.git" "master"
# Clone PFFFT
git_clone_if_not_exist "pffft" "https://bitbucket.org/jpommier/pffft.git" "master"

# Install google test
git_clone_if_not_exist "googletest" "https://github.com/google/googletest.git" "master"

# Install CMake Android/iOS toolchain support (optional for Android/iOS builds)
git_clone_if_not_exist "android-cmake" "https://github.com/taka-no-me/android-cmake.git" "master"
git_clone_if_not_exist "ios-cmake" "https://github.com/leetal/ios-cmake" "master"


