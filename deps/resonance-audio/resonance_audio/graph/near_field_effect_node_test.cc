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

#include "graph/near_field_effect_node.h"

#include <memory>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "dsp/distance_attenuation.h"
#include "dsp/stereo_panner.h"
#include "graph/buffered_source_node.h"
#include "node/sink_node.h"
#include "node/source_node.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

// Source Id for use with |BufferedSourceNode|.
const SourceId kSourceId = 0;

// Number of frames per buffer.
const size_t kFramesPerBuffer = kUnitRampLength;

// Sampling rate.
const int kSampleRate = 48000;

// Source distances.
const size_t kNumDistances = 5;
const float kDistances[kNumDistances] = {0.0f, 0.25f, 0.5f, 0.75f, 10.0f};

// Maximum expected gain change determines the number of buffers we need to wait
// for the output sample values to settle.
const size_t kMaxExpectedGainChange = 9;

// Expected offset due to near field processor delay compensation at 48000kHz.
const size_t kExpectedDelay = 31;

// Expected Dirac pulse attenuation due to shelf-filtering at 48000kHz.
const float KExpectedPeakReduction = 0.87319273f;

}  // namespace

TEST(NearFieldEffectNodeTest, VariousDistanceTest) {
  const size_t kDiracOffset = kFramesPerBuffer / 2;
  // We add one extra buffer since the Dirac is in the middle of the buffer.
  const size_t kBuffersToSettle = kMaxExpectedGainChange + 1;
  const auto kIdentityRotation = WorldRotation();
  SystemSettings system_settings(kNumStereoChannels, kFramesPerBuffer,
                                 kSampleRate);

  // Create the simple audio graph.
  auto near_field_effect_node =
      std::make_shared<NearFieldEffectNode>(kSourceId, system_settings);
  auto input_node = std::make_shared<BufferedSourceNode>(
      kSourceId, kNumMonoChannels, kFramesPerBuffer);
  auto output_node = std::make_shared<SinkNode>();
  near_field_effect_node->Connect(input_node);
  output_node->Connect(near_field_effect_node);
  auto parameters_manager = system_settings.GetSourceParametersManager();
  parameters_manager->Register(kSourceId);

  const AudioBuffer* output_buffer;
  for (size_t i = 0; i < kNumDistances; ++i) {
    const WorldPosition input_position(kDistances[i], 0.0f, 0.0f);
    // Loop till gain processors have reached steady state.
    for (size_t settle = 0; settle < kBuffersToSettle; ++settle) {
      AudioBuffer* const input_node_buffer =
          input_node->GetMutableAudioBufferAndSetNewBufferFlag();
      GenerateDiracImpulseFilter(kDiracOffset, &(*input_node_buffer)[0]);

      auto source_parameters =
          parameters_manager->GetMutableParameters(kSourceId);
      source_parameters->object_transform.position = input_position;
      source_parameters->near_field_gain = kMaxNearFieldEffectGain;
      // Retrieve the output.
      const auto& buffer_vector = output_node->ReadInputs();
      if (!buffer_vector.empty()) {
        EXPECT_EQ(buffer_vector.size(), 1U);
        output_buffer = buffer_vector.front();
      } else {
        output_buffer = nullptr;
      }
    }

    std::vector<float> stereo_pan_gains(kNumStereoChannels, 0.0f);
    // These methods are tested elsewhere. Their output will be used to
    // determine if the output from the |NearfieldEffectNode| is correct.
    WorldPosition relative_direction;
    GetRelativeDirection(system_settings.GetHeadPosition(), kIdentityRotation,
                         input_position, &relative_direction);
    const SphericalAngle source_direction =
        SphericalAngle::FromWorldPosition(relative_direction);
    CalculateStereoPanGains(source_direction, &stereo_pan_gains);
    const float near_field_gain = ComputeNearFieldEffectGain(
        system_settings.GetHeadPosition(), input_position);
    if (i < kNumDistances - 1) {
      EXPECT_FALSE(output_buffer == nullptr);
      EXPECT_NEAR(
          near_field_gain * stereo_pan_gains[0] * KExpectedPeakReduction,
          (*output_buffer)[0][kDiracOffset + kExpectedDelay], kEpsilonFloat);
      EXPECT_NEAR(
          near_field_gain * stereo_pan_gains[1] * KExpectedPeakReduction,
          (*output_buffer)[1][kDiracOffset + kExpectedDelay], kEpsilonFloat);
    } else {
      EXPECT_TRUE(output_buffer == nullptr);
    }
  }
}

}  // namespace vraudio
