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

#include "utils/test_util.h"

#include <cmath>
#include <string>

#include "third_party/googletest/googlemock/include/gmock/gmock.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

TEST(TestUtilTest, GenerateSineWave_SuccessfulGeneration) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 200;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    const float kFrequencyStart = 0.0f;
    const float kFrequencyStop = 2000.0f;
    const float kFrequencyStep = 100.0f;
    for (float frequency = kFrequencyStart; frequency <= kFrequencyStop;
         frequency += kFrequencyStep) {
      const int kSampleRate = 2000;
      AudioBuffer expected_signal(1, length);
      AudioBuffer::Channel& expected_signal_view = expected_signal[0];
      for (size_t i = 0; i < length; i++) {
        const float phase = static_cast<float>(i) * 2.0f *
                            static_cast<float>(M_PI) / kSampleRate * frequency;
        const float expected_value_float = std::sin(phase);
        expected_signal_view[i] = expected_value_float;
      }

      AudioBuffer sine_wave(1U, length);
      GenerateSineWave(frequency, kSampleRate, &sine_wave[0]);
      EXPECT_TRUE(CompareAudioBuffers(sine_wave[0], expected_signal_view,
                                      kEpsilonFloat));
    }
  }
}

TEST(TestUtilTest, GenerateSawToothSignal_SuccessfulGeneration) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 200;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    const size_t kToothLengthStart = 1;
    const size_t kToothLengthStop = 20;
    const size_t kToothLengthStep = 3;
    for (size_t tooth_length = kToothLengthStart;
         tooth_length <= kToothLengthStop; tooth_length += kToothLengthStep) {
      AudioBuffer expected_signal(1, length);
      AudioBuffer::Channel& expected_signal_view = expected_signal[0];
      for (size_t i = 0; i < length; i++) {
        const float expected_value = static_cast<float>(i % tooth_length) /
                                         static_cast<float>(tooth_length) *
                                         2.0f -
                                     1.0f;
        expected_signal_view[i] = expected_value;
      }

      AudioBuffer signal(1, length);
      GenerateSawToothSignal(tooth_length, &signal[0]);
      EXPECT_TRUE(
          CompareAudioBuffers(signal[0], expected_signal_view, kEpsilonFloat));
    }
  }
}

TEST(TestUtilTest, GenerateDiracImpulseFilterFloat_SuccessfulGeneration) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 100;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    const size_t kDelayStart = 1;
    const size_t kDelayStop = length - 1;
    const size_t kDelayStep = 3;
    for (size_t delay = kDelayStart; delay <= kDelayStop; delay += kDelayStep) {
      AudioBuffer expected_signal(1, length);
      GenerateSilence(&expected_signal[0]);
      expected_signal[0][delay] = 1.0f;

      AudioBuffer dirac_buffer(1, length);
      GenerateDiracImpulseFilter(delay, &dirac_buffer[0]);
      EXPECT_TRUE(CompareAudioBuffers(dirac_buffer[0], expected_signal[0],
                                      kEpsilonFloat));
    }
  }
}

TEST(TestUtilTest, GenerateIncreasingSignal_SuccessfulGeneration) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 200;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    AudioBuffer expected_signal(1, length);
    AudioBuffer::Channel& expected_signal_view = expected_signal[0];
    for (size_t i = 0; i < length; i++) {
      const float expected_value =
          static_cast<float>(i) / static_cast<float>(length) * 2.0f - 1.0f;
      expected_signal_view[i] = expected_value;
    }

    AudioBuffer signal(1, length);
    GenerateIncreasingSignal(&signal[0]);
    EXPECT_TRUE(
        CompareAudioBuffers(signal[0], expected_signal[0], kEpsilonFloat));
  }
}

TEST(TestUtilTest, ZeroCompare_SuccessfulZeroSignal) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 100;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    const size_t kZeroLengthStart = 0;
    const size_t kZeroLengthStop = length - 1;
    const size_t kZeroLengthStep = 3;
    for (size_t zero_length = kZeroLengthStart; zero_length <= kZeroLengthStop;
         zero_length += kZeroLengthStep) {
      AudioBuffer signal(1, length);
      AudioBuffer::Channel& signal_view = signal[0];
      GenerateSilence(&signal_view);
      for (size_t i = zero_length; i < length; i++) {
        signal_view[i] = 123.0f * static_cast<float>(i) + 1.0f;
      }
      const size_t result = ZeroCompare(signal_view, kEpsilonFloat);
      EXPECT_EQ(zero_length, result);
    }
  }
}

TEST(TestUtilTest, ZeroCompare_SuccessfulNonzeroSignal) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 100;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    const size_t kZeroLengthStart = 1;
    const size_t kZeroLengthStop = length - 1;
    const size_t kZeroLengthStep = 3;
    for (size_t zero_length = kZeroLengthStart; zero_length <= kZeroLengthStop;
         zero_length += kZeroLengthStep) {
      AudioBuffer signal(1, length);
      AudioBuffer::Channel& signal_view = signal[0];
      std::fill(signal_view.begin(), signal_view.end(), 100.0f);
      for (size_t i = 0; i < zero_length - 1; i++) {
        signal_view[i] = 0.0f;
        const size_t result = ZeroCompare(signal_view, kEpsilonFloat);
        EXPECT_NE(zero_length, result);
      }
    }
  }
}

TEST(TestUtilTest, DelayCompare_SuccessfulEqualDelay) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 100;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    AudioBuffer original_signal(1, length);
    AudioBuffer::Channel& original_signal_view = original_signal[0];

    GenerateIncreasingSignal(&original_signal_view);

    const size_t kDelayStart = 0;
    const size_t kDelayStop = length - 1;
    const size_t kDelayStep = 3;
    for (size_t delay = kDelayStart; delay <= kDelayStop; delay += kDelayStep) {
      AudioBuffer delayed_signal(1, length + delay);
      AudioBuffer::Channel& delayed_signal_view = delayed_signal[0];
      GenerateSilence(&delayed_signal_view);
      std::copy(original_signal_view.begin(), original_signal_view.end(),
                delayed_signal_view.begin() + delay);
      const size_t result = DelayCompare(
          original_signal_view, delayed_signal_view, delay, kEpsilonFloat);
      EXPECT_EQ(delayed_signal_view.size(), result);
    }
  }
}

TEST(TestUtilTest, DelayCompare_SuccessfulNotEqualDelay) {
  const size_t kLengthStart = 1;
  const size_t kLengthStop = 20;
  const size_t kLengthStep = 10;
  for (size_t length = kLengthStart; length <= kLengthStop;
       length += kLengthStep) {
    AudioBuffer original_signal(1, length);
    AudioBuffer::Channel& original_signal_view = original_signal[0];

    GenerateIncreasingSignal(&original_signal_view);

    const size_t kDelayStart = 1;
    const size_t kDelayStop = length - 1;
    const size_t kDelayStep = 3;
    for (size_t delay = kDelayStart; delay <= kDelayStop; delay += kDelayStep) {
      // Test altering first delayed element.
      {
        AudioBuffer delayed_signal(1, length + delay);
        AudioBuffer::Channel& delayed_signal_view = delayed_signal[0];
        std::copy(original_signal_view.begin(), original_signal_view.end(),
                  delayed_signal_view.begin() + delay);
        delayed_signal_view[delay] = -100.0f;
        const size_t result = DelayCompare(
            original_signal_view, delayed_signal_view, delay, kEpsilonFloat);
        EXPECT_NE(delayed_signal_view.size(), result);
      }
      // Test altering last delayed element.
      {
        AudioBuffer delayed_signal(1, length + delay);
        AudioBuffer::Channel& delayed_signal_view = delayed_signal[0];
        std::copy(original_signal_view.begin(), original_signal_view.end(),
                  delayed_signal_view.begin() + delay);
        delayed_signal_view[delayed_signal_view.size() - 1] = -100.0f;
        const size_t result = DelayCompare(
            original_signal_view, delayed_signal_view, delay, kEpsilonFloat);
        EXPECT_NE(delayed_signal_view.size(), result);
      }
    }
  }
}

}  // namespace

}  // namespace vraudio
