/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "platforms/common/room_effects_utils.h"

#include <memory>
#include <string>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "platforms/common/room_properties.h"

namespace vraudio {

namespace {

// Permitted error in the computed room effects coefficients against the
// expected values. This value is 1e-4 as expected values are rounded for
// readability.
const float kCoefficientsEpsilon = 1e-4f;

// Room position.
const float kRoomPosition[3] = {10.0f, 10.0f, 10.0f};

// Room dimensions (width x height x  depth) of a medium size hall (shoe-box
// model).
const float kRoomDimensions[3] = {5.85f, 4.65f, 15.0f};

// Reverb time adjustment parameters.
const float kBrightnessModifier = 0.0f;

const float kTimeModifier = 1.0f;

// Expected set of reflection coefficients for given materials.
const float kExpectedReflectionCoefficients[kNumRoomSurfaces] = {
    0.9381f, 0.9381f, 0.9679f, 0.9381f, 0.9381f, 0.9381f};

// Expected -3dB cut-off frequency of the early reflections low-pass filter.
const float kExpectedCutoffFrequency = 800.0f;

// Expected reverb time values.
const float kExpectedRt60Values[] = {0.47286f, 0.56928f, 0.69632f,
                                     0.89532f, 1.09418f, 1.90940f,
                                     1.67225f, 1.34293f, 0.56304f};

// Expected reverb gain value.
const float kExpectedGain = 0.045f;

}  // namespace

class RoomEffectsUtilsTest : public ::testing::Test {
 protected:
  RoomEffectsUtilsTest() {}
  // Virtual methods from ::testing::Test
  ~RoomEffectsUtilsTest() override {}
  void SetUp() override {
    // Set up room properties.
    room_properties_.position[0] = kRoomPosition[0];
    room_properties_.position[1] = kRoomPosition[1];
    room_properties_.position[2] = kRoomPosition[2];
    room_properties_.dimensions[0] = kRoomDimensions[0];
    room_properties_.dimensions[1] = kRoomDimensions[1];
    room_properties_.dimensions[2] = kRoomDimensions[2];
    // Set the material to 'Parquet on contrete' for the floor and 'Plywood
    // panel' for the walls and the ceiling.
    const std::vector<MaterialName> kMaterialNames = {
        MaterialName::kPlywoodPanel,      MaterialName::kPlywoodPanel,
        MaterialName::kParquetOnConcrete, MaterialName::kPlywoodPanel,
        MaterialName::kPlywoodPanel,      MaterialName::kPlywoodPanel};
    for (size_t i = 0; i < kNumRoomSurfaces; ++i) {
      room_properties_.material_names[i] = kMaterialNames[i];
    }
    room_properties_.reverb_brightness = kBrightnessModifier;
    room_properties_.reverb_time = kTimeModifier;
  }
  void TearDown() override {}

  RoomProperties room_properties_;
};

// Tests if the ComputeReflectionProperties() function returns the same
// reflection coefficient values as computed in MATLAB. Also, checks if other
// ReflectionProperties data members are set up correctly.
TEST_F(RoomEffectsUtilsTest, ComputeReflectionPropertiesTest) {
  const auto reflection_properties =
      ComputeReflectionProperties(room_properties_);

  // Check if the cutoff frequency is correct.
  EXPECT_NEAR(kExpectedCutoffFrequency, reflection_properties.cutoff_frequency,
              kCoefficientsEpsilon);
  // Check if the reflection coefficients for each surface are correct.
  for (size_t i = 0; i < kNumRoomSurfaces; ++i) {
    EXPECT_NEAR(kExpectedReflectionCoefficients[i],
                reflection_properties.coefficients[i], kCoefficientsEpsilon);
  }
}

// Tests if the ComputeReverbProperties() function returns the same reverb
// time values as computed in MATLAB. Also, checks if other ReverbProperties
// data members are set up correctly.
TEST_F(RoomEffectsUtilsTest, ComputeReverbPropertiesTest) {
  const auto reverb_properties = ComputeReverbProperties(room_properties_);

  // Check if the reverb time values in the octave bands are correct.
  for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
    EXPECT_NEAR(kExpectedRt60Values[band], reverb_properties.rt60_values[band],
                kCoefficientsEpsilon);
  }

  // Check the gain.
  EXPECT_NEAR(kExpectedGain, reverb_properties.gain, kCoefficientsEpsilon);
}

// Tests if the ComputeReflectionProperties() and ComputeReverbProperties()
// functions return reflection coefficients and RT60 values of 0 if there are no
// materials set.
TEST_F(RoomEffectsUtilsTest, ComputeRoomPropertiesNoMaterialsTest) {
  for (size_t i = 0; i < kNumRoomSurfaces; ++i) {
    room_properties_.material_names[i] = MaterialName::kTransparent;
  }
  const auto reflection_properties =
      ComputeReflectionProperties(room_properties_);
  const auto reverb_properties = ComputeReverbProperties(room_properties_);

  // Check if the reflection coefficient values are near 0.
  for (size_t surface = 0; surface < kNumRoomSurfaces; ++surface) {
    EXPECT_NEAR(0.0f, reflection_properties.coefficients[surface],
                kCoefficientsEpsilon);
  }
  // Check if the RT60 values are near 0.
  for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
    EXPECT_NEAR(0.0f, reverb_properties.rt60_values[band],
                kCoefficientsEpsilon);
  }
}

// Tests if the ComputeReverbProperties() function returns RT60 values of 0
// seconds if all the materials are set to 'transparent' (full absorption).
TEST_F(RoomEffectsUtilsTest, ComputeReverbPropertiesFullAbsorptionTest) {
  for (size_t i = 0; i < kNumRoomSurfaces; ++i) {
    room_properties_.material_names[i] = MaterialName::kTransparent;
  }
  const auto reverb_properties = ComputeReverbProperties(room_properties_);

  // Check if the RT60 values are near 0.
  for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
    EXPECT_NEAR(0.0f, reverb_properties.rt60_values[band],
                kCoefficientsEpsilon);
  }
  // Check the gain.
  EXPECT_NEAR(kExpectedGain, reverb_properties.gain, kCoefficientsEpsilon);
}

// Tests the room effects gain computation against the pre-computed result when
// source is inside the room.
TEST_F(RoomEffectsUtilsTest, ComputeRoomEffectsGainInsideRoomTest) {
  const WorldPosition kListenerPosition(0.0f, 0.5f, 0.0f);
  const WorldPosition kSourcePosition(0.5f, 0.3f, 0.2f);
  const WorldPosition kRoomPosition(0.5f, 0.5f, 0.5f);
  const WorldRotation kRoomRotation = WorldRotation();
  const WorldPosition kRoomDimensions(1.0f, 1.0f, 1.0f);
  const float kExpectedGain = 1.0f;

  const float gain = ComputeRoomEffectsGain(kSourcePosition, kRoomPosition,
                                            kRoomRotation, kRoomDimensions);
  EXPECT_NEAR(kExpectedGain, gain, kEpsilonFloat);
}

// Tests the room effects gain computation against the pre-computed result when
// source is outside the room.
TEST_F(RoomEffectsUtilsTest, ComputeRoomEffectsGainOutsideRoomTest) {
  const WorldPosition kListenerPosition(0.0f, 0.5f, 0.0f);
  const WorldPosition kSourcePosition(2.0f, 0.5f, 0.5f);
  const WorldPosition kRoomPosition(0.5f, 0.5f, 0.5f);
  const WorldRotation kRoomRotation = WorldRotation();
  const WorldPosition kRoomDimensions(1.0f, 1.0f, 1.0f);
  const float kExpectedGain = 0.25f;

  const float gain = ComputeRoomEffectsGain(kSourcePosition, kRoomPosition,
                                            kRoomRotation, kRoomDimensions);
  EXPECT_NEAR(kExpectedGain, gain, kEpsilonFloat);
}

}  // namespace vraudio
