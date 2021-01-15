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

#include "ambisonics/stereo_from_soundfield_converter.h"

#include <iterator>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "utils/planar_interleaved_conversion.h"

namespace vraudio {

namespace {

// Number of frames per buffer.
const size_t kFramesPerBuffer = 2;

// First order ambisonic signal in the AmbiX format (W, Y, Z, X), sound source
// in the front.
const float kFirstOrderSourceFront[] = {1.0f, 0.0f, 0.0f, 1.0f,
                                        1.0f, 0.0f, 0.0f, 1.0f};

// First order ambisonic signal in the AmbiX format (W, Y, Z, X), sound source
// to the left.
const float kFirstOrderSourceLeft[] = {1.0f, 1.0f, 0.0f, 0.0f,
                                       1.0f, 1.0f, 0.0f, 0.0f};

// Tests whether the conversion to stereo from soundfield results in equal
// signal in both L/R output channels when there is a single source in front of
// the soundfield.
TEST(StereoFromSoundfieldConverterTest, StereoFromSoundfieldFrontTest) {
  // Initialize the soundfield input buffer.
  const std::vector<float> soundfield_data(std::begin(kFirstOrderSourceFront),
                                           std::end(kFirstOrderSourceFront));
  AudioBuffer soundfield_buffer(
      kNumFirstOrderAmbisonicChannels,
      soundfield_data.size() / kNumFirstOrderAmbisonicChannels);
  FillAudioBuffer(soundfield_data, kNumFirstOrderAmbisonicChannels,
                  &soundfield_buffer);

  // Output buffer is stereo.
  AudioBuffer output(kNumStereoChannels,
                     soundfield_data.size() / kNumFirstOrderAmbisonicChannels);

  StereoFromSoundfield(soundfield_buffer, &output);

  // Test for near equality.
  ASSERT_EQ(kNumStereoChannels, output.num_channels());
  const AudioBuffer::Channel& output_channel_left = output[0];
  const AudioBuffer::Channel& output_channel_right = output[1];
  for (size_t frame = 0; frame < kFramesPerBuffer; ++frame) {
    EXPECT_NEAR(output_channel_left[frame], output_channel_right[frame],
                kEpsilonFloat);
  }
}

// Tests whether the conversion to stereo from soundfield, when the sound source
// in the soundfield is to the left, results in a signal only in the L output
// channel.
TEST(StereoFromSoundfieldConverterTest, StereoFromSoundfieldLeftTest) {
  // Initialize the soundfield input buffer.
  const std::vector<float> soundfield_data(std::begin(kFirstOrderSourceLeft),
                                           std::end(kFirstOrderSourceLeft));
  AudioBuffer soundfield_buffer(
      kNumFirstOrderAmbisonicChannels,
      soundfield_data.size() / kNumFirstOrderAmbisonicChannels);
  FillAudioBuffer(soundfield_data, kNumFirstOrderAmbisonicChannels,
                  &soundfield_buffer);

  // Output buffer is stereo.
  AudioBuffer output(kNumStereoChannels,
                     soundfield_data.size() / kNumFirstOrderAmbisonicChannels);

  StereoFromSoundfield(soundfield_buffer, &output);

  // Test for near equality.
  ASSERT_EQ(kNumStereoChannels, output.num_channels());
  const AudioBuffer::Channel& output_channel_left = output[0];
  const AudioBuffer::Channel& output_channel_right = output[1];
  for (size_t frame = 0; frame < kFramesPerBuffer; ++frame) {
    EXPECT_NEAR(output_channel_left[frame], 1.0f, kEpsilonFloat);
    EXPECT_NEAR(output_channel_right[frame], 0.0f, kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
