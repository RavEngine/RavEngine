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

#include "dsp/distance_attenuation.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

// Tests the logarithmic distance attenuation method against the pre-computed
// results.
TEST(DistanceAttenuationTest, ComputeLogarithmicDistanceAttenuationTest) {
  const WorldPosition kListenerPosition(0.0f, 3.0f, 0.0f);
  const WorldPosition kSourcePosition(0.0f, 0.0f, -4.0f);
  const float kMinDistance_low = 1.0f;
  const float kMinDistance_high = 10.0f;
  const float kMaxDistance_low = 3.0f;
  const float kMaxDistance_high = 500.0f;
  const float kExpectedAttenuation_a = 0.1983967f;
  const float kExpectedAttenuation_b = 0.0f;
  const float kExpectedAttenuation_c = 1.0f;

  float attenuation = ComputeLogarithmicDistanceAttenuation(
      kListenerPosition, kSourcePosition, kMinDistance_low, kMaxDistance_high);
  EXPECT_NEAR(kExpectedAttenuation_a, attenuation, kEpsilonFloat);

  // Test for the case where the source is beyond the maximum distance.
  attenuation = ComputeLogarithmicDistanceAttenuation(
      kListenerPosition, kSourcePosition, kMinDistance_low, kMaxDistance_low);
  EXPECT_NEAR(kExpectedAttenuation_b, attenuation, kEpsilonFloat);

  // Test for the case where the source is within the minimum distance.
  attenuation = ComputeLogarithmicDistanceAttenuation(
      kListenerPosition, kSourcePosition, kMinDistance_high, kMaxDistance_high);
  EXPECT_NEAR(kExpectedAttenuation_c, attenuation, kEpsilonFloat);
}

// Tests the linear distance attenuation method against the pre-computed
// results.
TEST(DistanceAttenuationTest, ComputeLinearDistanceAttenuationTest) {
  const WorldPosition kListenerPosition(0.0f, 3.0f, 0.0f);
  const WorldPosition kSourcePosition(0.0f, 0.0f, -4.0f);
  const float kMinDistance_low = 1.0f;
  const float kMinDistance_high = 10.0f;
  const float kMaxDistance_low = 3.0f;
  const float kMaxDistance_high = 8.0f;
  const float kExpectedAttenuation_a = 0.4285714f;
  const float kExpectedAttenuation_b = 0.0f;
  const float kExpectedAttenuation_c = 1.0f;

  float attenuation = ComputeLinearDistanceAttenuation(
      kListenerPosition, kSourcePosition, kMinDistance_low, kMaxDistance_high);
  EXPECT_NEAR(kExpectedAttenuation_a, attenuation, kEpsilonFloat);

  // Test for the case where the source is beyond the maximum distance.
  attenuation = ComputeLinearDistanceAttenuation(
      kListenerPosition, kSourcePosition, kMinDistance_low, kMaxDistance_low);
  EXPECT_NEAR(kExpectedAttenuation_b, attenuation, kEpsilonFloat);

  // Test for the case where the source is within the minimum distance.
  attenuation = ComputeLinearDistanceAttenuation(
      kListenerPosition, kSourcePosition, kMinDistance_high, kMaxDistance_high);
  EXPECT_NEAR(kExpectedAttenuation_c, attenuation, kEpsilonFloat);
}

// Tests the gain attenuations update method against the pre-computed results.
TEST(DistanceAttenuationTest, UpdateAttenuationParametersTest) {
  const float kMasterGain = 0.5f;
  const float kReflectionsGain = 0.5f;
  const float kReverbGain = 2.0f;
  const WorldPosition kListenerPosition(0.0f, 0.0f, 0.0f);
  const WorldPosition kSourcePosition(0.0f, 0.0f, 1.5f);
  const float kDistanceAttenuation = 0.2f;
  const float kRoomEffectsGain = 0.25f;

  // Initialize new |SourceParameters| with an explicit distance attenuation
  // value to avoid extra complexity.
  SourceParameters parameters;
  parameters.distance_rolloff_model = DistanceRolloffModel::kNone;
  parameters.distance_attenuation = kDistanceAttenuation;
  parameters.object_transform.position = kSourcePosition;
  parameters.room_effects_gain = kRoomEffectsGain;
  // Update the gain attenuation parameters.
  UpdateAttenuationParameters(kMasterGain, kReflectionsGain, kReverbGain,
                              kListenerPosition, &parameters);
  // Check the attenuation parameters against the pre-computed values.
  const size_t num_attenuations =
      static_cast<size_t>(AttenuationType::kNumAttenuationTypes);
  const float kExpectedAttenuations[num_attenuations] = {0.5f, 0.1f, 0.0125f,
                                                         0.25f};
  for (size_t i = 0; i < num_attenuations; ++i) {
    EXPECT_NEAR(kExpectedAttenuations[i], parameters.attenuations[i],
                kEpsilonFloat)
        << "Attenuation " << i;
  }
}

// Tests the near field effects gain computation method against the pre-computed
// results.
TEST(NearFieldEffectTest, ComputeNearFieldEffectTest) {
  const WorldPosition kListenerPosition(0.0f, 3.0f, 0.0f);
  const WorldPosition kSourcePosition_a(0.0f, 0.0f, -4.0f);
  const WorldPosition kSourcePosition_b(0.0f, 2.5f, 0.0f);
  const float kExpectedGain_a = 0.0f;
  const float kExpectedGain_b = 1.0f;

  float gain = ComputeNearFieldEffectGain(kListenerPosition, kSourcePosition_a);
  EXPECT_NEAR(kExpectedGain_a, gain, kEpsilonFloat);
  gain = ComputeNearFieldEffectGain(kListenerPosition, kSourcePosition_b);
  EXPECT_NEAR(kExpectedGain_b, gain, kEpsilonFloat);
}

}  // namespace

}  // namespace vraudio
