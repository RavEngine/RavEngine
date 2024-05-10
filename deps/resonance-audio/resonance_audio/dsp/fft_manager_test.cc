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

#include "dsp/fft_manager.h"

#include <cstdlib>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/logging.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

const float kInverseFftEpsilon = 2e-5f;

// This tests that the |FreqFromTimeDomain| and |TimeFromFreqDomain|
// functions are the inverse of one another for a number of fft sizes and signal
// types.
TEST(FftManagerTest, FftIfftTest) {
  // Generate a test signal.
  const size_t kNumSignalTypes = 3;
  const size_t kNumBufferLengths = 10;
  const size_t kBufferLengths[kNumBufferLengths] = {31,  32,  63,  64,  127,
                                                    128, 255, 256, 511, 512};

  for (size_t length_idx = 0; length_idx < kNumBufferLengths; length_idx++) {
    for (size_t type = 0; type < kNumSignalTypes; ++type) {
      AudioBuffer time_signal(kNumMonoChannels, kBufferLengths[length_idx]);
      for (size_t i = 0; i < kBufferLengths[length_idx]; ++i) {
        switch (type) {
          case 0:
            time_signal[0][i] = static_cast<float>(i) /
                                static_cast<float>(kBufferLengths[length_idx]);
            break;
          case 1:
            time_signal[0][i] = std::cos(static_cast<float>(i));
            break;
          case 2:
            time_signal[0][i] = static_cast<float>(i % 2) * -0.5f;
            break;
        }
      }
      AudioBuffer freq_signal(kNumMonoChannels,
                              NextPowTwo(kBufferLengths[length_idx]) * 2);
      AudioBuffer output(kNumMonoChannels, kBufferLengths[length_idx]);
      output.Clear();

      FftManager fft_manager(kBufferLengths[length_idx]);

      fft_manager.FreqFromTimeDomain(time_signal[0], &freq_signal[0]);
      fft_manager.TimeFromFreqDomain(freq_signal[0], &output[0]);
      fft_manager.ApplyReverseFftScaling(&output[0]);

      for (size_t i = 0; i < kBufferLengths[length_idx]; ++i) {
        EXPECT_NEAR(output[0][i], time_signal[0][i], kEpsilonFloat);
      }
    }
  }
}

// Tests that the result from an inverse FFT is the same whether it is written
// into a buffer of |frames_per_buffer_| or |fft_size_| in length.
TEST(FftManagerTest, ReverseFftOutputSizeTest) {
  const size_t kFramesPerBuffer = 32;
  AudioBuffer freq_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  freq_buffer.Clear();
  AudioBuffer time_buffer_short(kNumMonoChannels, kFramesPerBuffer);
  time_buffer_short.Clear();
  AudioBuffer time_buffer_long(kNumMonoChannels, 2 * kFramesPerBuffer);
  time_buffer_long.Clear();
  AudioBuffer input_buffer(kNumMonoChannels, kFramesPerBuffer);

  std::srand(0);
  for (auto& sample : input_buffer[0]) {
    sample = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
  }

  FftManager fft_manager(kFramesPerBuffer);
  fft_manager.FreqFromTimeDomain(input_buffer[0], &freq_buffer[0]);
  fft_manager.TimeFromFreqDomain(freq_buffer[0], &time_buffer_short[0]);
  fft_manager.ApplyReverseFftScaling(&time_buffer_short[0]);
  fft_manager.TimeFromFreqDomain(freq_buffer[0], &time_buffer_long[0]);
  fft_manager.ApplyReverseFftScaling(&time_buffer_long[0]);

  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR(time_buffer_short[0][i], time_buffer_long[0][i], kEpsilonFloat);
    EXPECT_NEAR(time_buffer_long[0][i + kFramesPerBuffer], 0.0f,
                kInverseFftEpsilon);
  }
}

// Tests that a frequency domain buffer can be transformed into a canonical
// format and back.
TEST(FftManagerTest, PffftFormatToCanonicalFormatTest) {
  const size_t kFramesPerBuffer = 32;
  AudioBuffer time_buffer(kNumMonoChannels, kFramesPerBuffer);
  time_buffer.Clear();
  time_buffer[0][0] = 1.0f;
  time_buffer[0][1] = 1.0f;
  AudioBuffer freq_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  freq_buffer.Clear();
  AudioBuffer reordered_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  reordered_buffer.Clear();
  AudioBuffer final_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  reordered_buffer.Clear();

  FftManager fft_manager(kFramesPerBuffer);
  fft_manager.FreqFromTimeDomain(time_buffer[0], &freq_buffer[0]);

  fft_manager.GetCanonicalFormatFreqBuffer(freq_buffer[0],
                                           &reordered_buffer[0]);
  fft_manager.GetPffftFormatFreqBuffer(reordered_buffer[0], &final_buffer[0]);

  for (size_t i = 0; i < kFramesPerBuffer * 2; ++i) {
    EXPECT_NEAR(final_buffer[0][i], freq_buffer[0][i], kEpsilonFloat);
  }
}

// Tests that for a scaled kronecker delta, the magnitude response will be flat
// and equal to the absolute magnitude of the kronecker.
TEST(FftManagerTest, MagnitudeTest) {
  const size_t kFramesPerBuffer = 32;
  const size_t kMagnitudeLength = kFramesPerBuffer + 1;
  FftManager fft_manager(kFramesPerBuffer);
  AudioBuffer time_buffer(kNumMonoChannels, kFramesPerBuffer);
  time_buffer.Clear();
  AudioBuffer freq_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  AudioBuffer reordered_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  AudioBuffer magnitude_buffer(kNumMonoChannels, kMagnitudeLength);
  const std::vector<float> magnitudes = {1.0f, 2.0f, 3.0f, -1.0f, -2.0f, -3.0f};

  for (auto& magnitude : magnitudes) {
    time_buffer[0][0] = magnitude;
    fft_manager.FreqFromTimeDomain(time_buffer[0], &freq_buffer[0]);
    fft_manager.GetCanonicalFormatFreqBuffer(freq_buffer[0],
                                             &reordered_buffer[0]);
    fft_manager.MagnitudeFromCanonicalFreqBuffer(reordered_buffer[0],
                                                 &magnitude_buffer[0]);
    for (size_t sample = 0; sample < kMagnitudeLength; ++sample) {
      // Check its correct to within 0.5%.
      const float kErrEpsilon = 5e-3f;
      const float expected = std::abs(magnitude);
      EXPECT_NEAR(magnitude_buffer[0][sample], expected,
                  kErrEpsilon * expected);
    }
  }
}

// Tests that conversion from Canonical frequency domain data to phase and
// magnitude spectra and back results in an output equal to the input.
TEST(FftManagerTest, FreqFromMagnitudePhase) {
  const size_t kFramesPerBuffer = 16;
  const size_t kMagnitudePhaseLength = kFramesPerBuffer + 1;
  FftManager fft_manager(kFramesPerBuffer);
  AudioBuffer time_buffer(kNumMonoChannels, kFramesPerBuffer);
  time_buffer.Clear();
  time_buffer[0][0] = 0.5f;
  time_buffer[0][1] = 1.0f;
  AudioBuffer freq_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  AudioBuffer reordered_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  AudioBuffer phase_buffer(kNumMonoChannels, kMagnitudePhaseLength);
  AudioBuffer magnitude_buffer(kNumMonoChannels, kMagnitudePhaseLength);
  fft_manager.FreqFromTimeDomain(time_buffer[0], &freq_buffer[0]);
  fft_manager.GetCanonicalFormatFreqBuffer(freq_buffer[0],
                                           &reordered_buffer[0]);
  fft_manager.MagnitudeFromCanonicalFreqBuffer(reordered_buffer[0],
                                               &magnitude_buffer[0]);

  // Calculate the phase.
  phase_buffer[0][0] = 0.0f;
  for (size_t i = 1, j = 2; i < kFramesPerBuffer; ++i, j += 2) {
    phase_buffer[0][i] = std::atan2(reordered_buffer[0][j + 1] /*imag*/,
                                    reordered_buffer[0][j] /*real*/);
  }
  phase_buffer[0][kFramesPerBuffer] = kPi;

  fft_manager.CanonicalFreqBufferFromMagnitudeAndPhase(
      magnitude_buffer[0], phase_buffer[0], &freq_buffer[0]);

  for (size_t sample = 0; sample < kFramesPerBuffer * 2; ++sample) {
    // Check its correct to within 0.5%.
    const float kErrEpsilon = 5e-3f;
    EXPECT_NEAR(freq_buffer[0][sample], reordered_buffer[0][sample],
                kErrEpsilon * std::abs(reordered_buffer[0][sample]));
  }
}

// Tests that conversion from  phase and magnitude spectra and back results in
// an output equal to that from sine and cosine phase, using SIMD on arm.
TEST(FftManagerTest, FMagnitudePhaseAndSineCosinePhase) {
  const size_t kFramesPerBuffer = 16;
  const size_t kMagnitudePhaseLength = kFramesPerBuffer + 1;
  FftManager fft_manager(kFramesPerBuffer);
  AudioBuffer freq_buffer_one(kNumMonoChannels, 2 * kFramesPerBuffer);
  AudioBuffer freq_buffer_two(kNumMonoChannels, 2 * kFramesPerBuffer);
  AudioBuffer phase_buffer(kNumMonoChannels, kMagnitudePhaseLength);
  AudioBuffer sin_phase_buffer(kNumMonoChannels, kMagnitudePhaseLength);
  AudioBuffer cos_phase_buffer(kNumMonoChannels, kMagnitudePhaseLength);
  AudioBuffer magnitude_buffer(kNumMonoChannels, kMagnitudePhaseLength);

  std::fill(magnitude_buffer[0].begin(), magnitude_buffer[0].end(), 2.0f);
  phase_buffer[0] = std::vector<float>(
      {0.4720f, 1.6100f, -1.9831f, 0.7569f, 0.2799f, -1.1481f, -0.3807f,
       0.3008f, 3.1416f, 2.4314f, -1.1851f, 2.6645f, 0.6369f, -0.0554f, 0.6275f,
       -0.1799f, 1.2345f});
  for (size_t i = 0; i < phase_buffer.num_frames(); ++i) {
    sin_phase_buffer[0][i] = std::sin(phase_buffer[0][i]);
    cos_phase_buffer[0][i] = std::cos(phase_buffer[0][i]);
  }

  fft_manager.CanonicalFreqBufferFromMagnitudeAndPhase(
      magnitude_buffer[0], phase_buffer[0], &freq_buffer_one[0]);
  fft_manager.CanonicalFreqBufferFromMagnitudeAndSinCosPhase(
      0, /* phase_offset */
      magnitude_buffer[0], sin_phase_buffer[0], cos_phase_buffer[0],
      &freq_buffer_two[0]);

  for (size_t i = 0; i < kFramesPerBuffer * 2; ++i) {
    EXPECT_NEAR(freq_buffer_one[0][i], freq_buffer_two[0][i], kEpsilonFloat);
  }
}

// Tests that the correct scaling factor is applied consistently across a time
// domain buffer.
TEST(FftManagerTest, ReverseScalingTest) {
  const size_t kFramesPerBuffer = 128;
  const float kExpectedScale = 1.0f / static_cast<float>(2 * kFramesPerBuffer);

  AudioBuffer buffer(kNumMonoChannels, kFramesPerBuffer);
  for (auto& sample : buffer[0]) {
    sample = 1.0f;
  }
  FftManager fft_manager(kFramesPerBuffer);

  fft_manager.ApplyReverseFftScaling(&buffer[0]);
  for (auto& sample : buffer[0]) {
    EXPECT_NEAR(sample, kExpectedScale, kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
