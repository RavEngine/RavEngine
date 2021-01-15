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

#include <algorithm>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/logging.h"
#include "node/processing_node.h"
#include "node/sink_node.h"
#include "node/source_node.h"

namespace vraudio {

namespace {

// Number of channels in test buffers.
static const size_t kNumChannels = 5;

// Number of frames in test buffers.
static const size_t kNumFrames = 7;

// Helper method to compare two audio buffers.
bool CompareAudioBuffer(const AudioBuffer& buffer_a,
                        const AudioBuffer& buffer_b) {
  if (buffer_a.num_channels() != buffer_b.num_channels() ||
      buffer_a.num_frames() != buffer_b.num_frames()) {
    return false;
  }
  for (size_t channel = 0; channel < buffer_a.num_channels(); ++channel) {
    const AudioBuffer::Channel& channel_a = buffer_a[channel];
    const AudioBuffer::Channel& channel_b = buffer_b[channel];
    for (size_t frame = 0; frame < buffer_a.num_frames(); ++frame) {
      if (channel_a[frame] != channel_b[frame]) {
        return false;
      }
    }
  }
  return true;
}

// Helper method to generate a test AudioBuffer.
std::unique_ptr<AudioBuffer> GenerateTestAudioBuffer(float factor) {
  std::unique_ptr<AudioBuffer> new_buffer(
      new AudioBuffer(kNumChannels, kNumFrames));
  for (size_t channel = 0; channel < kNumChannels; ++channel) {
    std::fill((*new_buffer)[channel].begin(), (*new_buffer)[channel].end(),
              static_cast<float>(channel) * factor);
  }
  return new_buffer;
}

// Simple audio source node that generates AudioData buffers.
class MySourceNode : public SourceNode {
 public:
  MySourceNode(bool output_empty_buffer, bool* node_deletion_flag)
      : output_empty_buffer_(output_empty_buffer),
        node_deletion_flag_(node_deletion_flag),
        audio_buffer_(GenerateTestAudioBuffer(1.0f)) {}
  ~MySourceNode() final {
    if (node_deletion_flag_ != nullptr) {
      *node_deletion_flag_ = true;
    }
  }

 protected:
  // AudioProcess methods outputs kTestAudioData data.
  const AudioBuffer* AudioProcess() override {
    if (output_empty_buffer_) {
      return nullptr;
    }
    return audio_buffer_.get();
  }

 private:
  const bool output_empty_buffer_;
  bool* node_deletion_flag_;

  std::unique_ptr<AudioBuffer> audio_buffer_;
};

// Simple audio processing node that reads from a single input and passes the
// data to the output.
class MyProcessingNode : public ProcessingNode {
 public:
  MyProcessingNode(bool process_on_empty_input, bool* audio_process_called_flag,
                   bool* node_deletion_flag)
      : process_on_empty_input_(process_on_empty_input),
        audio_process_called_flag_(audio_process_called_flag),
        node_deletion_flag_(node_deletion_flag) {
    EnableProcessOnEmptyInput(process_on_empty_input_);
  }

  ~MyProcessingNode() final {
    if (node_deletion_flag_ != nullptr) {
      *node_deletion_flag_ = true;
    }
  }

 protected:
  const AudioBuffer* AudioProcess(const NodeInput& input) override {
    if (audio_process_called_flag_ != nullptr) {
      *audio_process_called_flag_ = true;
    }

    if (!process_on_empty_input_) {
      EXPECT_GT(input.GetInputBuffers().size(), 0U);
    } else {
      if (input.GetInputBuffers().empty()) {
        return nullptr;
      }
    }

    return input.GetInputBuffers()[0];
  }

 private:
  const bool process_on_empty_input_;
  bool* const audio_process_called_flag_;
  bool* const node_deletion_flag_;
};

// Simple audio mixer node that connects to multiple nodes and outputs the sum
// of all inputs.
class MyAudioMixerNode : public ProcessingNode {
 public:
  explicit MyAudioMixerNode(bool* node_deletion_flag)
      : node_deletion_flag_(node_deletion_flag) {}
  ~MyAudioMixerNode() final {
    if (node_deletion_flag_ != nullptr) {
      *node_deletion_flag_ = true;
    }
  }

 protected:
  // AudioProcess performs an element-wise sum from all inputs and outputs a new
  // AudioData buffer.
  const AudioBuffer* AudioProcess(const NodeInput& input) override {
    const auto& input_buffers = input.GetInputBuffers();
    output_data_ = *input_buffers[0];

    // Iterate over all inputs and add its data to |output_data|.
    for (size_t buffer = 1; buffer < input_buffers.size(); ++buffer) {
      const AudioBuffer* accumulate_data = input_buffers[buffer];
      for (size_t channel = 0; channel < accumulate_data->num_channels();
           ++channel) {
        const AudioBuffer::Channel& accumulate_channel =
            (*accumulate_data)[channel];
        AudioBuffer::Channel* output_channel = &output_data_[channel];
        EXPECT_EQ(accumulate_channel.size(), output_channel->size());
        for (size_t frame = 0; frame < accumulate_channel.size(); ++frame) {
          (*output_channel)[frame] += accumulate_channel[frame];
        }
      }
    }
    return &output_data_;
  }

 private:
  bool* node_deletion_flag_;
  AudioBuffer output_data_;
};

// Simple audio sink node that expects a single input.
class MySinkNode : public SinkNode {
 public:
  explicit MySinkNode(bool* node_deletion_flag)
      : node_deletion_flag_(node_deletion_flag) {}

  ~MySinkNode() final {
    if (node_deletion_flag_ != nullptr) {
      *node_deletion_flag_ = true;
    }
  }

 private:
  bool* node_deletion_flag_;
};

// Tests a chain of an |SinkNode|, |ProcessingNode| and
// |SourceNode|.
TEST(AudioNodesTest, SourceProcessingSinkConnectionTest) {
  static const bool kOutputEmptyBuffer = false;
  static const bool kEnableProcessOnEmptyInput = true;

  auto source_node =
      std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
  auto processing_node = std::make_shared<MyProcessingNode>(
      kEnableProcessOnEmptyInput, nullptr, nullptr);
  auto sink_node = std::make_shared<MySinkNode>(nullptr);

  // Create chain of nodes.
  sink_node->Connect(processing_node);
  processing_node->Connect(source_node);

  // Test output data.
  const auto& data = sink_node->ReadInputs();
  EXPECT_GT(data.size(), 0U);
  EXPECT_TRUE(CompareAudioBuffer(*data[0], *GenerateTestAudioBuffer(1.0f)));
}

// Tests a chain of an |SinkNode| and |AudioMixerNode| connected to two
// |SourceNodes|.
TEST(AudioNodesTest, MixerProcessingConnectionTest) {
  static const bool kOutputEmptyBuffer = false;
  auto source_node_a =
      std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
  auto source_node_b =
      std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
  auto mixer_node = std::make_shared<MyAudioMixerNode>(nullptr);
  auto sink_node = std::make_shared<MySinkNode>(nullptr);

  // Create chain of nodes.
  sink_node->Connect(mixer_node);
  mixer_node->Connect(source_node_a);
  mixer_node->Connect(source_node_b);

  // Test output data.
  const auto& data = sink_node->ReadInputs();
  EXPECT_GT(data.size(), 0U);
  EXPECT_TRUE(CompareAudioBuffer(*data[0], *GenerateTestAudioBuffer(2.0f)));
}

// Tests if ProcessingNode::AudioProcess() calls are skipped in case of
// empty input buffers.
TEST(AudioNodesTest, SkipProcessingOnEmptyInputTest) {
  static const bool kEnableProcessOnEmptyInput = true;

  bool audio_process_called = false;

  // Tests that ProcessingNode::AudioProcess() is called in case source
  // nodes do generate output.
  {
    static const bool kOutputEmptyBuffer = false;
    auto source_node_a =
        std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
    auto source_node_b =
        std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
    auto processing_node = std::make_shared<MyProcessingNode>(
        kEnableProcessOnEmptyInput, &audio_process_called, nullptr);
    auto sink_node = std::make_shared<MySinkNode>(nullptr);

    // Create chain of nodes.
    sink_node->Connect(processing_node);
    processing_node->Connect(source_node_a);
    processing_node->Connect(source_node_b);

    EXPECT_FALSE(audio_process_called);
    const auto& data = sink_node->ReadInputs();
    EXPECT_TRUE(audio_process_called);
    EXPECT_GT(data.size(), 0U);
  }

  audio_process_called = false;

  // Tests that ProcessingNode::AudioProcess() is *not* called in case
  // source nodes do *not* generate output.
  {
    static const bool kOutputEmptyBuffer = true;
    auto source_node_a =
        std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
    auto source_node_b =
        std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
    auto processing_node = std::make_shared<MyProcessingNode>(
        false, &audio_process_called, nullptr);
    auto sink_node = std::make_shared<MySinkNode>(nullptr);

    // Create chain of nodes.
    sink_node->Connect(processing_node);
    processing_node->Connect(source_node_a);
    processing_node->Connect(source_node_b);

    EXPECT_FALSE(audio_process_called);
    const auto& data = sink_node->ReadInputs();
    EXPECT_FALSE(audio_process_called);
    EXPECT_EQ(data.size(), 0U);
  }
}

// Tests a chain of an |SinkNode|, |ProcessingNode| and
// |SourceNode| and runs the node clean-up procedure with sources being
// *not* marked with end-of-stream.
TEST(AudioNodesTest, NodeCleanUpWithoutMarkEndOfStreamCallTest) {
  static const bool kEnableProcessOnEmptyInput = true;

  bool source_node_deleted = false;
  bool processing_node_deleted = false;
  bool sink_node_deleted = false;

  auto sink_node = std::make_shared<MySinkNode>(&sink_node_deleted);

  {
    // Create a source and processing node and connect it to sink node.
    auto source_node = std::make_shared<MySourceNode>(
        /*output_empty_buffer=*/false, &source_node_deleted);
    auto processing_node = std::make_shared<MyProcessingNode>(
        kEnableProcessOnEmptyInput, nullptr, &processing_node_deleted);

    // Connect nodes.
    sink_node->Connect(processing_node);
    processing_node->Connect(source_node);

    // End-of-stream is not marked in source node.
    // source_node->MarkEndOfStream();
  }

  EXPECT_FALSE(source_node_deleted);
  EXPECT_FALSE(processing_node_deleted);
  EXPECT_FALSE(sink_node_deleted);

  sink_node->CleanUp();

  EXPECT_FALSE(source_node_deleted);
  EXPECT_FALSE(processing_node_deleted);
  EXPECT_FALSE(sink_node_deleted);
}

// Tests a chain of an SinkNode, ProcessingNode and SourceNode
// and runs the node clean-up procedure with sources being marked with
// end-of-stream.
TEST(AudioNodesTest, NodeCleanUpTest) {
  static const bool kEnableProcessOnEmptyInput = true;

  bool source_node_deleted = false;
  bool processing_node_deleted = false;
  bool sink_node_deleted = false;

  auto sink_node = std::make_shared<MySinkNode>(&sink_node_deleted);

  {
    // Create a source and processing node and connect it to sink node.
    auto source_node = std::make_shared<MySourceNode>(
        /*output_empty_buffer=*/false, &source_node_deleted);
    auto processing_node = std::make_shared<MyProcessingNode>(
        kEnableProcessOnEmptyInput, nullptr, &processing_node_deleted);

    // Connect nodes.
    sink_node->Connect(processing_node);
    processing_node->Connect(source_node);

    // End of stream is marked in source node. Do not expect any data anymore.
    source_node->MarkEndOfStream();
  }

  EXPECT_FALSE(source_node_deleted);
  EXPECT_FALSE(processing_node_deleted);
  EXPECT_FALSE(sink_node_deleted);

  sink_node->CleanUp();

  EXPECT_TRUE(source_node_deleted);
  EXPECT_TRUE(processing_node_deleted);
  EXPECT_FALSE(sink_node_deleted);
}

// Tests ProcessingNode::EnableProcessOnEmptyInput().
TEST(AudioNodesTest, ProcessOnEmptyInputFlagTest) {
  bool audio_process_called = false;

  static const bool kEnableProcessOnEmptyInput = true;
  static const bool kOutputEmptyBuffer = true;

  auto source_node =
      std::make_shared<MySourceNode>(kOutputEmptyBuffer, nullptr);
  auto processing_node = std::make_shared<MyProcessingNode>(
      kEnableProcessOnEmptyInput, &audio_process_called, nullptr);
  auto sink_node = std::make_shared<MySinkNode>(nullptr);

  // Create chain of nodes.
  sink_node->Connect(processing_node);
  processing_node->Connect(source_node);

  EXPECT_FALSE(audio_process_called);
  const auto& data = sink_node->ReadInputs();
  EXPECT_TRUE(audio_process_called);
  EXPECT_EQ(data.size(), 0U);
}

// Tests a chain of an |SinkNode| and |AudioMixerNode| without a |SourceNode|.
TEST(AudioNodesTest, MissingSourceConnectionTest) {
  auto mixer_node = std::make_shared<MyAudioMixerNode>(nullptr);
  auto sink_node = std::make_shared<MySinkNode>(nullptr);

  // Create chain of nodes.
  sink_node->Connect(mixer_node);

  // Test output data.
  const auto& data = sink_node->ReadInputs();
  EXPECT_EQ(data.size(), 0U);
}

}  // namespace

}  // namespace vraudio
