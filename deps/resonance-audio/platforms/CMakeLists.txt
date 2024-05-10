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

if (BUILD_VST_MONITOR_PLUGIN)
    add_subdirectory(vst)
endif (BUILD_VST_MONITOR_PLUGIN)

if (BUILD_FMOD_PLUGIN)
    add_subdirectory(fmod)
endif (BUILD_FMOD_PLUGIN)

if (BUILD_WWISE_AUTHORING_PLUGIN OR BUILD_WWISE_SOUND_ENGINE_PLUGIN)
    add_subdirectory(wwise)
endif (BUILD_WWISE_AUTHORING_PLUGIN OR BUILD_WWISE_SOUND_ENGINE_PLUGIN)

if (BUILD_UNITY_PLUGIN OR BUILD_GEOMETRICAL_ACOUSTICS_TESTS)
    add_subdirectory(unity)
endif (BUILD_UNITY_PLUGIN OR BUILD_GEOMETRICAL_ACOUSTICS_TESTS)
