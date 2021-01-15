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

#include "graph/gain_node.h"

#include <iterator>
#include <memory>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "graph/buffered_source_node.h"
#include "node/sink_node.h"

namespace vraudio {

namespace {

// Values to initialize a |SystemSettings| instance.
const size_t kNumFrames = 4;
const size_t kSampleRate = 44100;

// Source id.
const SourceId kSourceId = 1;

const float kInputData[kNumFrames] = {1.0f, 2.0f, 3.0f, 4.0f};

// Provides unit tests for |GainNode|.
class GainNodeTest : public ::testing::Test {
 protected:
  GainNodeTest()
      : input_data_(std::begin(kInputData), std::end(kInputData)),
        system_settings_(kNumMonoChannels, kNumFrames, kSampleRate) {}

  void SetUp() override {
    // Tests will use |AttenuationType::kInput| which directly returns the input
    // gain value in order to avoid extra complexity.
    gain_node_ = std::make_shared<GainNode>(
        kSourceId, kNumMonoChannels, AttenuationType::kInput, system_settings_);
    input_buffer_node_ = std::make_shared<BufferedSourceNode>(
        kSourceId, kNumMonoChannels, kNumFrames);
    gain_node_->Connect(input_buffer_node_);
    output_node_ = std::make_shared<SinkNode>();
    output_node_->Connect(gain_node_);
    // Register the source parameters.
    system_settings_.GetSourceParametersManager()->Register(kSourceId);
  }

  // Helper method to create a new input buffer.
  //
  // @return Mono audio buffer filled with test data.
  std::unique_ptr<AudioBuffer> CreateInputBuffer() {
    std::unique_ptr<AudioBuffer> buffer(
        new AudioBuffer(kNumMonoChannels, kNumFrames));
    (*buffer)[0] = input_data_;
    return buffer;
  }

  // Helper method that generates a node graph and returns the processed output.
  //
  // @param input_gain Input gain value to be processed.
  // @return Processed output buffer.

  const AudioBuffer* ProcessGainNode(float input_gain) {
    // Create a new audio buffer.
    auto input = CreateInputBuffer();
    // Update the input gain parameter.
    auto parameters =
        system_settings_.GetSourceParametersManager()->GetMutableParameters(
            kSourceId);
    parameters->attenuations[AttenuationType::kInput] = input_gain;
    // Process the buffer.
    AudioBuffer* const input_node_buffer =
        input_buffer_node_->GetMutableAudioBufferAndSetNewBufferFlag();
    *input_node_buffer = *input;

    const std::vector<const AudioBuffer*>& outputs = output_node_->ReadInputs();
    if (!outputs.empty()) {
      DCHECK_EQ(outputs.size(), 1U);
      return outputs.front();
    }
    return nullptr;
  }

 private:
  std::vector<float> input_data_;

  std::shared_ptr<GainNode> gain_node_;
  std::shared_ptr<BufferedSourceNode> input_buffer_node_;
  std::shared_ptr<SinkNode> output_node_;

  SystemSettings system_settings_;
};

// Tests that the gain node returns the expected output buffers with different
// gain values.
TEST_F(GainNodeTest, GainTest) {
  // nullptr should be returned when the gain value is zero from the start.
  auto output = ProcessGainNode(0.0f);

  EXPECT_TRUE(output == nullptr);

  // A valid output buffer should be returned when the gain value is non-zero.
  output = ProcessGainNode(0.5f);

  EXPECT_FALSE(output == nullptr);

  // A valid output buffer should be returned even when the gain value is zero
  // while gain processor interpolation.
  output = ProcessGainNode(0.0f);

  EXPECT_FALSE(output == nullptr);

  // nullptr should be returned after the interpolation is completed.
  for (size_t i = 0; i < kUnitRampLength; ++i) {
    output = ProcessGainNode(0.0f);
  }

  EXPECT_TRUE(output == nullptr);
}

}  // namespace

}  // namespace vraudio
