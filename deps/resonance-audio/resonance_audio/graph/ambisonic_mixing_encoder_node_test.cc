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

#include "graph/ambisonic_mixing_encoder_node.h"

#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "ambisonics/ambisonic_lookup_table.h"
#include "ambisonics/utils.h"
#include "api/resonance_audio_api.h"
#include "base/constants_and_types.h"
#include "base/object_transform.h"
#include "config/source_config.h"
#include "graph/buffered_source_node.h"
#include "node/sink_node.h"
#include "node/source_node.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

// Number of frames per input buffer.
const size_t kFramesPerBuffer = 16;

// Simulated system sample rate.
const int kSampleRate = 48000;

}  // namespace

// Provides unit tests for |AmbisonicMixingEncoderNode|.
class AmbisonicMixingEncoderNodeTest
    : public ::testing::TestWithParam<SourceGraphConfig> {
 protected:
  AmbisonicMixingEncoderNodeTest()
      : system_settings_(kNumStereoChannels, kFramesPerBuffer, kSampleRate),
        lookup_table_(kMaxSupportedAmbisonicOrder) {}

  void SetUp() override {
    const auto source_config = GetParam();
    ambisonic_order_ = source_config.ambisonic_order;
    ambisonic_mixing_encoder_node_ =
        std::make_shared<AmbisonicMixingEncoderNode>(
            system_settings_, lookup_table_, ambisonic_order_);
  }

  const AudioBuffer* ProcessMultipleInputs(size_t num_sources,
                                           const WorldPosition& position,
                                           float spread_deg) {
    // Create the node graph, adding input nodes to the Ambisonic Mixing Encoder
    // Node.
    buffered_source_nodes_.clear();
    auto parameters_manager = system_settings_.GetSourceParametersManager();
    for (size_t i = 0; i < num_sources; ++i) {
      buffered_source_nodes_.emplace_back(std::make_shared<BufferedSourceNode>(
          static_cast<SourceId>(i) /*source id*/, kNumMonoChannels,
          kFramesPerBuffer));
      parameters_manager->Register(static_cast<SourceId>(i));
    }
    const AudioBuffer* output_buffer = nullptr;

    for (auto& input_node : buffered_source_nodes_) {
      ambisonic_mixing_encoder_node_->Connect(input_node);
    }
    auto output_node = std::make_shared<SinkNode>();
    output_node->Connect(ambisonic_mixing_encoder_node_);

    // Input data containing unit pulses.
    const std::vector<float> kInputData(kFramesPerBuffer, 1.0f);

    for (size_t index = 0; index < buffered_source_nodes_.size(); ++index) {
      AudioBuffer* input_buffer =
          buffered_source_nodes_[index]
              ->GetMutableAudioBufferAndSetNewBufferFlag();
      (*input_buffer)[0] = kInputData;
      auto source_parameters = parameters_manager->GetMutableParameters(
          static_cast<SourceId>(index));
      source_parameters->object_transform.position = position;
      source_parameters->spread_deg = spread_deg;
    }

    const std::vector<const AudioBuffer*>& buffer_vector =
        output_node->ReadInputs();
    if (!buffer_vector.empty()) {
      DCHECK_EQ(buffer_vector.size(), 1U);
      output_buffer = buffer_vector.front();
    }

    return output_buffer;
  }

  SystemSettings system_settings_;
  int ambisonic_order_;
  AmbisonicLookupTable lookup_table_;
  std::shared_ptr<AmbisonicMixingEncoderNode> ambisonic_mixing_encoder_node_;
  std::vector<std::shared_ptr<BufferedSourceNode>> buffered_source_nodes_;
};

// Tests that a number of sound objects encoded in the same direction are
// correctly combined into an output buffer.
TEST_P(AmbisonicMixingEncoderNodeTest, TestEncodeAndMix) {
  // Number of sources to encode and mix.
  const size_t kNumSources = 4;
  // Minimum angular source spread of 0 ensures that no gain correction
  // coefficients are to be applied to the Ambisonic encoding coefficients.
  const float kSpreadDeg = 0.0f;
  // Arbitrary world position of sound sources corresponding to the 36 degrees
  // azimuth and 18 degrees elevation.
  const WorldPosition kPosition =
      WorldPosition(-0.55901699f, 0.30901699f, -0.76942088f);
  // Expected Ambisonic output for a single source at the above position (as
  // generated using /matlab/ambisonics/ambix/ambencode.m Matlab function):
  const std::vector<float> kExpectedSingleSourceOutput = {
      1.0f,         0.55901699f,  0.30901699f,  0.76942088f,
      0.74498856f,  0.29920441f,  -0.35676274f, 0.41181955f,
      0.24206145f,  0.64679299f,  0.51477443f,  -0.17888019f,
      -0.38975424f, -0.24620746f, 0.16726035f,  -0.21015578f};

  const AudioBuffer* output_buffer =
      ProcessMultipleInputs(kNumSources, kPosition, kSpreadDeg);

  const size_t num_channels = GetNumPeriphonicComponents(ambisonic_order_);

  for (size_t i = 0; i < num_channels; ++i) {
    EXPECT_NEAR(
        kExpectedSingleSourceOutput[i] * static_cast<float>(kNumSources),
        (*output_buffer)[i][kFramesPerBuffer - 1], kEpsilonFloat);
  }
}

INSTANTIATE_TEST_CASE_P(TestParameters, AmbisonicMixingEncoderNodeTest,
                        testing::Values(BinauralLowQualityConfig(),
                                        BinauralMediumQualityConfig(),
                                        BinauralHighQualityConfig()));

}  // namespace vraudio
