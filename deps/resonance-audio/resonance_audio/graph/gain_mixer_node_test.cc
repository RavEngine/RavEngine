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

#include "graph/gain_mixer_node.h"

#include <algorithm>
#include <limits>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/logging.h"
#include "graph/buffered_source_node.h"
#include "node/sink_node.h"
#include "node/source_node.h"

namespace vraudio {

namespace {

// Values to initialize a |SystemSettings| instance.
const size_t kNumFrames = 4;
const size_t kSampleRate = 44100;

// Helper class to detect deletion.
class DeletionDetector {
 public:
  explicit DeletionDetector(bool* node_deletion_flag)
      : node_deletion_flag_(node_deletion_flag) {}
  ~DeletionDetector() {
    if (node_deletion_flag_ != nullptr) {
      *node_deletion_flag_ = true;
    }
  }

 private:
  bool* node_deletion_flag_;
};

// Wraps |SourceNode| to detect its deletion.
class MySourceNode : public SourceNode, DeletionDetector {
 public:
  explicit MySourceNode(bool* node_deletion_flag)
      : DeletionDetector(node_deletion_flag) {}

 protected:
  const AudioBuffer* AudioProcess() final { return nullptr; }
};

// Wraps |GainMixerNode| to detect its deletion.
class MyGainMixerNode : public GainMixerNode, DeletionDetector {
 public:
  MyGainMixerNode(bool* node_deletion_flag,
                  const AttenuationType& attenuation_type,
                  const SystemSettings& system_settings)
      : GainMixerNode(attenuation_type, system_settings, kNumMonoChannels),
        DeletionDetector(node_deletion_flag) {}
};

// Wraps |SinkNode| to detect its deletion.
class MySinkNode : public SinkNode, DeletionDetector {
 public:
  explicit MySinkNode(bool* node_deletion_flag)
      : DeletionDetector(node_deletion_flag) {}
};

// Tests that the |GainMixerNode| keeps connected at the moment all of its
// sources are removed.
TEST(AudioNodesTest, CleanUpOnEmptyInputTest) {
  bool source_node_deleted = false;
  bool gain_mixer_node_deleted = false;
  bool sink_node_deleted = false;

  SystemSettings system_settings(kNumMonoChannels, kNumFrames, kSampleRate);
  auto sink_node = std::make_shared<MySinkNode>(&sink_node_deleted);

  {
    // Create a source and mixer node and connect it to sink node.
    auto source_node = std::make_shared<MySourceNode>(&source_node_deleted);
    auto gain_mixer_node = std::make_shared<MyGainMixerNode>(
        &gain_mixer_node_deleted, AttenuationType::kInput, system_settings);

    // Connect nodes.
    sink_node->Connect(gain_mixer_node);
    gain_mixer_node->Connect(source_node);

    // End of stream is marked in source node. Do not expect any data anymore.
    source_node->MarkEndOfStream();
  }

  EXPECT_FALSE(source_node_deleted);
  EXPECT_FALSE(gain_mixer_node_deleted);
  EXPECT_FALSE(sink_node_deleted);

  sink_node->CleanUp();

  EXPECT_TRUE(source_node_deleted);
  EXPECT_FALSE(gain_mixer_node_deleted);
  EXPECT_FALSE(sink_node_deleted);
}

// Provides unit tests for |GainMixerNode|.
class GainMixerNodeTest : public ::testing::Test {
 protected:
  GainMixerNodeTest()
      : system_settings_(kNumMonoChannels, kNumFrames, kSampleRate) {}

  // Helper method to create a new input buffer.
  //
  // @return Mono audio buffer filled with test data.
  std::unique_ptr<AudioBuffer> CreateInputBuffer(
      const std::vector<float>& input_data) {
    auto buffer = std::unique_ptr<AudioBuffer>(
        new AudioBuffer(kNumMonoChannels, input_data.size()));
    (*buffer)[0] = input_data;
    return buffer;
  }

  // Helper method that generates a node graph and returns the processed
  // output.
  //
  // @param num_inputs Number of input buffers to be processed.
  void CreateGraph(size_t num_inputs) {
    // Tests will use |AttenuationType::kInput| which directly returns the
    // local
    // gain value in order to avoid extra complexity.
    gain_mixer_node_ = std::make_shared<GainMixerNode>(
        AttenuationType::kInput, system_settings_, kNumMonoChannels);

    output_node_ = std::make_shared<SinkNode>();
    output_node_->Connect(gain_mixer_node_);

    buffered_source_nodes_.resize(num_inputs);
    auto parameters_manager = system_settings_.GetSourceParametersManager();
    for (size_t i = 0; i < num_inputs; ++i) {
      const auto source_id = static_cast<SourceId>(i);
      buffered_source_nodes_[i] = std::make_shared<BufferedSourceNode>(
          source_id, kNumMonoChannels, kNumFrames);
      gain_mixer_node_->Connect(buffered_source_nodes_[i]);
      parameters_manager->Register(source_id);
    }
  }

  // Processes input buuffers with gains returning the mixed output.
  //
  // @param input_gains Gains to be processed.
  // @param input_buffers Buffers to be processed.
  // @return Processed output buffer.
  const AudioBuffer* Process(
      float input_gain, const std::vector<std::vector<float>>& input_buffers) {
    DCHECK_EQ(buffered_source_nodes_.size(), input_buffers.size());
    for (size_t i = 0; i < input_buffers.size(); ++i) {
      const auto source_id = static_cast<SourceId>(i);
      auto input = CreateInputBuffer(input_buffers[i]);
      // Set the input gain.
      auto parameters =
          system_settings_.GetSourceParametersManager()->GetMutableParameters(
              source_id);
      parameters->attenuations[AttenuationType::kInput] = input_gain;
      // Process the buffer.
      AudioBuffer* const input_node_buffer =
          buffered_source_nodes_[i]->GetMutableAudioBufferAndSetNewBufferFlag();
      *input_node_buffer = *input;
    }
    const std::vector<const AudioBuffer*>& outputs = output_node_->ReadInputs();
    if (!outputs.empty()) {
      DCHECK_EQ(outputs.size(), 1U);
      return outputs.front();
    }
    return nullptr;
  }

  // System settings.
  SystemSettings system_settings_;

  // Component nodes for the simple audio graph.
  std::shared_ptr<GainMixerNode> gain_mixer_node_;
  std::vector<std::shared_ptr<BufferedSourceNode>> buffered_source_nodes_;
  std::shared_ptr<SinkNode> output_node_;
};

// Tests that the |GainMixerNode| returns the expected output buffers with
// different gain values.
TEST_F(GainMixerNodeTest, GainTest) {
  const float kGain = 0.5f;
  const std::vector<std::vector<float>> inputs({{1.0f, 1.0f, 1.0f, 1.0f},
                                                {2.0f, 2.0f, 2.0f, 2.0f},
                                                {3.0f, 3.0f, 3.0f, 3.0f},
                                                {4.0f, 4.0f, 4.0f, 4.0f}});
  // Zero buffer should be returned when the gain value's zero from the start.
  CreateGraph(inputs.size());
  auto output = Process(0.0f, inputs);
  for (size_t i = 0; i < inputs[0].size(); ++i) {
    EXPECT_NEAR((*output)[0][i], 0.0f, kEpsilonFloat);
  }

  // A valid output buffer should be returned when the gain value is non-zero.
  output = Process(kGain, inputs);
  EXPECT_NEAR((*output)[0][0], 0.0f, kEpsilonFloat);
  for (size_t i = 1; i < inputs[0].size(); ++i) {
    EXPECT_FALSE(std::abs((*output)[0][i]) <=
                 std::numeric_limits<float>::epsilon());
  }

  // Correct values should be returned after gain processor interpolation.
  for (size_t i = 0; i < kUnitRampLength / 2; ++i) {
    output = Process(kGain, inputs);
  }
  const float output_value =
      kGain * (inputs[0][0] + inputs[1][0] + inputs[2][0] + inputs[3][0]);
  for (size_t i = 0; i < inputs[0].size(); ++i) {
    EXPECT_NEAR((*output)[0][i], output_value, kEpsilonFloat);
  }

  // A valid output buffer should be returned even when the gain value is zero
  // while gain processor interpolation.
  output = Process(0.0f, inputs);
  for (size_t i = 0; i < inputs[0].size(); ++i) {
    EXPECT_NE((*output)[0][i], 0.0f);
  }

  // Zero buffer should be returned after the interpolation is completed.
  for (size_t i = 0; i < kUnitRampLength / 2; ++i) {
    output = Process(0.0f, inputs);
  }
  for (size_t i = 0; i < inputs[0].size(); ++i) {
    EXPECT_NEAR((*output)[0][i], 0.0f, kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
