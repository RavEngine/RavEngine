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

#include "ambisonics/hoa_rotator.h"

#include <algorithm>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "ambisonics/ambisonic_codec_impl.h"
#include "base/constants_and_types.h"
#include "utils/planar_interleaved_conversion.h"

namespace vraudio {

namespace {

using testing::tuple;
using testing::Values;

const int kAmbisonicOrder = 3;

// Rotation angle used in the test.
const float kAngleDegrees = 90.0f;

// Initial, arbitrary direction of the encoded soundfield source.
const SphericalAngle kInitialSourceAngle =
    SphericalAngle::FromDegrees(22.0f, 33.0f);

// Expected direction of the sound source rotated by |kAngleDegrees| against the
// right facing axis (x in the world coordinate system).
const SphericalAngle kXrotatedSourceAngle =
    SphericalAngle::FromDegrees(150.021778639249f, 51.0415207997462f);

// Expected direction of the sound source rotated by |kAngleDegrees| against the
// upward facing axis (y in the world coordinate system).
const SphericalAngle kYrotatedSourceAngle =
    SphericalAngle::FromDegrees(112.0f, 33.0f);

// Expected direction of the sound source rotated by |kAngleDegrees| against the
// back facing axis (z in the world coordinate system).
const SphericalAngle kZrotatedSourceAngle =
    SphericalAngle::FromDegrees(35.0077312770459f, -18.3108067341351f);

// Rotation interpolation interval in terms of frames (used by the HoaRotator).
const size_t kSlerpFrameInterval = 32;

class HoaRotatorTest : public ::testing::Test {
 protected:
  HoaRotatorTest() {}

  // Virtual methods from ::testing::Test
  ~HoaRotatorTest() override {}

  void SetUp() override {
    // Initialize the third order soundfield rotator.
    hoa_rotator_ = std::unique_ptr<HoaRotator>(new HoaRotator(kAmbisonicOrder));
  }

  void TearDown() override {}

  // Test method which creates a HOA soundfield buffer, rotates it by
  // |rotation_angle| against |rotation_axis|, and compares the output to the
  // reference soundfield buffer (where the source is already spatialized at the
  // |expected_source_angle|).
  void CompareRotatedAndReferenceSoundfields(
      const std::vector<float>& input_data, float rotation_angle,
      const WorldPosition& rotation_axis,
      const SphericalAngle& expected_source_angle) {
    AudioBuffer input_buffer(1, input_data.size());
    FillAudioBuffer(input_data, 1, &input_buffer);
    // Initialize a third order mono codec with the |kInitialSourceAngle|. This
    // will be used to obtain the rotated soundfield.
    std::unique_ptr<MonoAmbisonicCodec<>> rotated_source_mono_codec(
        new MonoAmbisonicCodec<>(kAmbisonicOrder, {kInitialSourceAngle}));
    // Initialize a third order mono codec with the expected source angle. This
    // will be used as a reference for the rotated soundfield.
    std::unique_ptr<MonoAmbisonicCodec<>> reference_source_mono_codec(
        new MonoAmbisonicCodec<>(kAmbisonicOrder, {expected_source_angle}));
    // Generate soundfield buffers representing sound sources at required
    // angles.
    AudioBuffer encoded_rotated_buffer(kNumThirdOrderAmbisonicChannels,
                                       input_data.size());
    AudioBuffer encoded_reference_buffer(kNumThirdOrderAmbisonicChannels,
                                         input_data.size());
    rotated_source_mono_codec->EncodeBuffer(input_buffer,
                                            &encoded_rotated_buffer);
    reference_source_mono_codec->EncodeBuffer(input_buffer,
                                              &encoded_reference_buffer);
    // Rotate the test soundfield by |rotation_angle| degrees wrt the given
    // |rotation_axis|.
    const WorldRotation rotation = WorldRotation(
        AngleAxisf(rotation_angle * kRadiansFromDegrees, rotation_axis));
    const bool result = hoa_rotator_->Process(rotation, encoded_rotated_buffer,
                                              &encoded_rotated_buffer);
    EXPECT_TRUE(result);
    // Check if the sound source in the reference and rotated buffers are in the
    // same direction.
    // If the buffer size is more than |kSlerpFrameInterval|, due to
    // interpolation, we expect that the last |kSlerpFrameInterval| frames have
    // undergone the full rotation.
    // If the buffer size is less than |kSlerpFrameInterval|, because no
    // interpolation is applied, the rotated soundfield should result from
    // frame 0.
    for (size_t channel = 0; channel < encoded_rotated_buffer.num_channels();
         ++channel) {
      const int num_frames =
          static_cast<int>(encoded_rotated_buffer[channel].size());
      const int interval = static_cast<int>(kSlerpFrameInterval);
      const int frames_to_compare =
          (num_frames % interval) ? (num_frames % interval) : interval;
      const int start_frame = std::max(0, num_frames - frames_to_compare);
      ASSERT_LT(start_frame, num_frames);
      for (int frame = start_frame; frame < num_frames; ++frame) {
        EXPECT_NEAR(encoded_rotated_buffer[channel][frame],
                    encoded_reference_buffer[channel][frame], kEpsilonFloat);
      }
    }
  }

  // Higher Order Ambisonic rotator instance.
  std::unique_ptr<HoaRotator> hoa_rotator_;
};

// Tests that no rotation is aplied if |kRotationQuantizationRad| is not
// exceeded.
TEST_F(HoaRotatorTest, RotationThresholdTest) {
  const size_t kFramesPerBuffer = 16;
  const std::vector<float> kInputVector(
      kFramesPerBuffer * kNumThirdOrderAmbisonicChannels, 1.0f);
  AudioBuffer input_buffer(kNumThirdOrderAmbisonicChannels, kFramesPerBuffer);
  FillAudioBuffer(kInputVector, kNumThirdOrderAmbisonicChannels, &input_buffer);
  AudioBuffer output_buffer(kNumThirdOrderAmbisonicChannels, kFramesPerBuffer);
  const WorldRotation kSmallRotation =
      WorldRotation(1.0f, 0.001f, 0.001f, 0.001f);
  const WorldRotation kLargeRotation = WorldRotation(1.0f, 0.1f, 0.1f, 0.1f);
  EXPECT_FALSE(
      hoa_rotator_->Process(kSmallRotation, input_buffer, &output_buffer));
  EXPECT_TRUE(
      hoa_rotator_->Process(kLargeRotation, input_buffer, &output_buffer));
}

typedef tuple<WorldPosition, SphericalAngle> TestParams;
class HoaAxesRotationTest : public HoaRotatorTest,
                            public ::testing::WithParamInterface<TestParams> {};

// Tests third order soundfield rotation against the x, y and z axis using long
// input buffers (>> slerp interval).
TEST_P(HoaAxesRotationTest, CompareWithExpectedAngleLongBuffer) {
  const WorldPosition& rotation_axis = ::testing::get<0>(GetParam());
  const SphericalAngle& expected_angle = ::testing::get<1>(GetParam());
  const size_t kLongFramesPerBuffer = 512;
  const std::vector<float> kLongInputData(kLongFramesPerBuffer, 1.0f);
  CompareRotatedAndReferenceSoundfields(kLongInputData, kAngleDegrees,
                                        rotation_axis, expected_angle);
}

// Tests third order soundfield rotation against the x, y and z axes using short
// input buffers (< slerp interval).
TEST_P(HoaAxesRotationTest, CompareWithExpectedAngleShortBuffer) {
  const WorldPosition& rotation_axis = ::testing::get<0>(GetParam());
  const SphericalAngle& expected_angle = ::testing::get<1>(GetParam());
  const size_t kShortFramesPerBuffer = kSlerpFrameInterval / 2;
  const std::vector<float> kShortInputData(kShortFramesPerBuffer, 1.0f);
  CompareRotatedAndReferenceSoundfields(kShortInputData, kAngleDegrees,
                                        rotation_axis, expected_angle);
}

// Tests third order soundfield rotation against the x, y and z axes using
// buffer sizes that are (> slerp interval) and not integer multiples of
// the slerp interval.
TEST_P(HoaAxesRotationTest, CompareWithExpectedAngleOddBufferSizes) {
  const WorldPosition& rotation_axis = ::testing::get<0>(GetParam());
  const SphericalAngle& expected_angle = ::testing::get<1>(GetParam());
  const size_t frames_per_buffer = kSlerpFrameInterval + 3U;
  const std::vector<float> kShortInputData(frames_per_buffer, 1.0f);
  CompareRotatedAndReferenceSoundfields(kShortInputData, kAngleDegrees,
                                        rotation_axis, expected_angle);
}

INSTANTIATE_TEST_CASE_P(
    TestParameters, HoaAxesRotationTest,
    Values(TestParams({1.0f, 0.0f, 0.0f}, kXrotatedSourceAngle),
           TestParams({0.0f, 1.0f, 0.0f}, kYrotatedSourceAngle),
           TestParams({0.0f, 0.0f, 1.0f}, kZrotatedSourceAngle)));

}  // namespace

}  // namespace vraudio
