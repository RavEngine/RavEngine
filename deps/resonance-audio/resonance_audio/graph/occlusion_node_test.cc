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

#include "graph/occlusion_node.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

// Number of frames per buffer.
const size_t kFramesPerBuffer = 256;

// Sampling rate.
const int kSampleRate = 48000;

// Generated sawtooth length for test buffers.
const size_t kSawtoothLength = 16;

// Source id.
const SourceId kSourceId = 1;

}  // namespace

class OcclusionNodeTest : public ::testing::Test {
 protected:
  OcclusionNodeTest()
      : system_settings_(kNumStereoChannels, kFramesPerBuffer, kSampleRate) {}

  void SetUp() override {
    system_settings_.GetSourceParametersManager()->Register(kSourceId);
  }

  // Function which wraps the buffer passed in a vector for processing and
  // returns the output of the occlusion nodes AudioProcess method.
  const AudioBuffer* GetProcessedData(const AudioBuffer* input_buffer,
                                      OcclusionNode* occlusion_node) {
    std::vector<const AudioBuffer*> input_buffers;
    input_buffers.push_back(input_buffer);
    return occlusion_node->AudioProcess(
        ProcessingNode::NodeInput(input_buffers));
  }

  // Returns a pointer to the parameters of the source.
  SourceParameters* GetParameters() {
    return system_settings_.GetSourceParametersManager()->GetMutableParameters(
        kSourceId);
  }

  // System settings.
  SystemSettings system_settings_;
};

// Test to ensure that no effect is made on a buffer of input if both occlusion
// and self-occlusion are absent.
TEST_F(OcclusionNodeTest, NoOcclusionTest) {
  OcclusionNode occlusion_processor(kSourceId, system_settings_);

  AudioBuffer input(1, kFramesPerBuffer);
  input.set_source_id(kSourceId);
  GenerateSawToothSignal(kSawtoothLength, &input[0]);

  const AudioBuffer* output = GetProcessedData(&input, &occlusion_processor);
  const bool buffers_identical =
      CompareAudioBuffers((*output)[0], input[0], kEpsilonFloat);
  EXPECT_TRUE(buffers_identical);
}

// Test to ensure that a more heavily occluded object shows a lower energy
// output.
TEST_F(OcclusionNodeTest, OcclusionTest) {
  OcclusionNode occlusion_processor_1(kSourceId, system_settings_);
  OcclusionNode occlusion_processor_2(kSourceId, system_settings_);

  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer);
  GenerateSawToothSignal(kSawtoothLength, &input[0]);
  AudioBuffer input_1;
  input_1 = input;
  input_1.set_source_id(kSourceId);
  AudioBuffer input_2;
  input_2 = input;
  input_2.set_source_id(kSourceId);

  SourceParameters* parameters = GetParameters();

  parameters->occlusion_intensity = 0.5f;
  const AudioBuffer* output_1 =
      GetProcessedData(&input_1, &occlusion_processor_1);
  parameters->occlusion_intensity = 1.0f;
  const AudioBuffer* output_2 =
      GetProcessedData(&input_2, &occlusion_processor_2);
  const double output_1_energy = CalculateSignalRms((*output_1)[0]);
  const double output_2_energy = CalculateSignalRms((*output_2)[0]);
  const double input_energy = CalculateSignalRms(input[0]);

  EXPECT_LT(output_1_energy, input_energy);
  EXPECT_LT(output_2_energy, output_1_energy);
}

// Test to ensure that setting a non-omnidirectional listener directivity
// pattern shows a lower energy output when the listener orientation is pointing
// away from a source.
TEST_F(OcclusionNodeTest, ListenerDirectivityTest) {
  OcclusionNode occlusion_processor(kSourceId, system_settings_);

  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer);
  input.set_source_id(kSourceId);
  GenerateSawToothSignal(kSawtoothLength, &input[0]);

  // Set a hyper-cardioid shaped listener directivity for the source input.
  SourceParameters* parameters = GetParameters();
  parameters->listener_directivity_alpha = 0.5f;
  parameters->listener_directivity_order = 2.0f;
  // Set listener position to one meter away from the origin in Z axis. This is
  // required for the listener directivity properties to take effect.
  system_settings_.SetHeadPosition(WorldPosition(0.0f, 0.0f, 1.0f));

  const double input_energy = CalculateSignalRms(input[0]);
  // Process input with identity listener orientation.
  const AudioBuffer* output_default =
      GetProcessedData(&input, &occlusion_processor);
  const double output_default_energy = CalculateSignalRms((*output_default)[0]);
  // Process input with 90 degrees rotated listener orientation about Y axis.
  system_settings_.SetHeadRotation(
      WorldRotation(0.0f, kInverseSqrtTwo, 0.0f, kInverseSqrtTwo));
  const AudioBuffer* output_rotated =
      GetProcessedData(&input, &occlusion_processor);
  const double output_rotated_energy = CalculateSignalRms((*output_rotated)[0]);

  // Test if the output energy is lower when the listener was rotated away from
  // the source.
  EXPECT_NEAR(output_default_energy, input_energy, kEpsilonFloat);
  EXPECT_LT(output_rotated_energy, output_default_energy);
}

// Test to ensure that a more heavily self occluded source shows a lower energy
// output.
TEST_F(OcclusionNodeTest, SourceDirectivityTest) {
  OcclusionNode occlusion_processor_1(kSourceId, system_settings_);
  OcclusionNode occlusion_processor_2(kSourceId, system_settings_);

  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer);
  input.set_source_id(kSourceId);
  GenerateSawToothSignal(kSawtoothLength, &input[0]);
  AudioBuffer input_1;
  input_1 = input;
  AudioBuffer input_2;
  input_2 = input;

  SourceParameters* parameters = GetParameters();

  parameters->directivity_alpha = 0.25f;
  parameters->directivity_order = 2.0f;
  const AudioBuffer* output_1 =
      GetProcessedData(&input_1, &occlusion_processor_1);

  parameters->directivity_order = 4.0f;
  parameters->directivity_alpha = 0.25f;
  const AudioBuffer* output_2 =
      GetProcessedData(&input_2, &occlusion_processor_2);

  const double output_1_energy = CalculateSignalRms((*output_1)[0]);
  const double output_2_energy = CalculateSignalRms((*output_2)[0]);
  const double input_energy = CalculateSignalRms(input[0]);

  EXPECT_LT(output_1_energy, input_energy);
  EXPECT_LT(output_2_energy, output_1_energy);
}

}  // namespace vraudio
