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

#include "ambisonics/ambisonic_binaural_decoder.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "dsp/fft_manager.h"

namespace vraudio {

namespace {

// Generates sample audio data where the first sample is 0 and each consecutive
// sample is incremented by 0.001. Then it moves to the next channel and
// continues to increment the samples.
std::vector<std::vector<float>> GenerateAudioData(size_t num_channels,
                                                  size_t num_samples) {
  std::vector<std::vector<float>> input_data(num_channels,
                                             std::vector<float>(num_samples));
  float sample_value = 0.0f;
  const float increment = 0.001f;
  for (size_t channel = 0; channel < num_channels; ++channel) {
    for (size_t sample = 0; sample < num_samples; ++sample) {
      input_data[channel][sample] = sample_value;
      sample_value += increment;
    }
  }
  return input_data;
}

// Number of frames in each audio input/output buffer.
const size_t kFramesPerBuffer = 18;

}  // namespace

// Tests whether binaural docoding of Ambisonic input using short HRIR filters
// (shorter than the number of frames per buffer) gives correct results.
TEST(AmbisonicBinauralDecoderTest, ShortFilterTest) {
  const std::vector<std::vector<float>> kInputData =
      GenerateAudioData(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer);
  const std::vector<float> kExpectedOutputLeft = {
      0.0042840000f, 0.0087780003f, 0.013486000f, 0.018412000f, 0.023560001f,
      0.028934000f,  0.034538001f,  0.040376000f, 0.046452001f, 0.052770000f,
      0.059333999f,  0.066147998f,  0.073215999f, 0.080541998f, 0.088129997f,
      0.095983997f,  0.10410800f,   0.10638600f};
  const std::vector<float> kExpectedOutputRight = {
      0.0036720000f, 0.0074840002f, 0.011438000f, 0.015536000f, 0.019780001f,
      0.024172001f,  0.028713999f,  0.033408001f, 0.038256001f, 0.043260001f,
      0.048422001f,  0.053743999f,  0.059227999f, 0.064875998f, 0.070689999f,
      0.076672003f,  0.082823999f,  0.084252000f};
  const std::vector<std::vector<float>> kHrirDataShort =
      GenerateAudioData(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer - 1);

  AudioBuffer sh_hrirs(kHrirDataShort.size(), kHrirDataShort[0].size());
  sh_hrirs = kHrirDataShort;

  AudioBuffer input(kInputData.size(), kInputData[0].size());
  input = kInputData;

  AudioBuffer output(kNumStereoChannels, kInputData[0].size());
  output.Clear();

  FftManager fft_manager(kFramesPerBuffer);
  AmbisonicBinauralDecoder decoder(sh_hrirs, kFramesPerBuffer, &fft_manager);
  decoder.Process(input, &output);

  for (size_t sample = 0; sample < kFramesPerBuffer; ++sample) {
    EXPECT_NEAR(kExpectedOutputLeft[sample], output[0][sample], kEpsilonFloat);
    EXPECT_NEAR(kExpectedOutputRight[sample], output[1][sample], kEpsilonFloat);
  }
}

// Tests whether binaural docoding of Ambisonic input using HRIR filters
// with the same size as frames per buffer gives correct results.
TEST(AmbisonicBinauralDecoderTest, SameSizeFilterTest) {
  const std::vector<std::vector<float>> kInputData =
      GenerateAudioData(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer);
  const std::vector<float> kExpectedOutputLeft = {
      0.0045360001f, 0.0092879999f, 0.014260001f, 0.019455999f, 0.024879999f,
      0.030536000f,  0.036428001f,  0.042560000f, 0.048935998f, 0.055560000f,
      0.062436000f,  0.069568001f,  0.076959997f, 0.084615998f, 0.092540003f,
      0.10073600f,   0.10920800f,   0.11796000f};
  const std::vector<float> kExpectedOutputRight = {
      0.0038880000f, 0.0079199998f, 0.012098000f, 0.016424000f, 0.020900000f,
      0.025528001f,  0.030309999f,  0.035248000f, 0.040344000f, 0.045600001f,
      0.051018000f,  0.056600001f,  0.062348001f, 0.068264000f, 0.074349999f,
      0.080608003f,  0.087040000f,  0.093648002f};
  const std::vector<std::vector<float>> kHrirDataSame =
      GenerateAudioData(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer);

  AudioBuffer sh_hrirs(kHrirDataSame.size(), kHrirDataSame[0].size());
  sh_hrirs = kHrirDataSame;

  AudioBuffer input(kInputData.size(), kInputData[0].size());
  input = kInputData;

  AudioBuffer output(kNumStereoChannels, kInputData[0].size());
  output.Clear();

  FftManager fft_manager(kFramesPerBuffer);
  AmbisonicBinauralDecoder decoder(sh_hrirs, kFramesPerBuffer, &fft_manager);
  decoder.Process(input, &output);

  for (size_t sample = 0; sample < kFramesPerBuffer; ++sample) {
    EXPECT_NEAR(kExpectedOutputLeft[sample], output[0][sample], kEpsilonFloat);
    EXPECT_NEAR(kExpectedOutputRight[sample], output[1][sample], kEpsilonFloat);
  }
}

// Tests whether binaural docoding of Ambisonic input using long HRIR filters
// (longer than the number of frames per buffer) gives correct results.
TEST(AmbisonicBinauralDecoderTest, LongSizeFilterTest) {
  const std::vector<std::vector<float>> kInputData =
      GenerateAudioData(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer);
  const std::vector<float> kExpectedOutputLeft = {
      0.0047880001f, 0.0097980006f, 0.015034000f, 0.020500001f, 0.026200000f,
      0.032138001f,  0.038318001f,  0.044744000f, 0.051419999f, 0.058350001f,
      0.065537997f,  0.072987996f,  0.080704004f, 0.088689998f, 0.096950002f,
      0.10548800f,   0.11430800f,   0.12341400f};
  const std::vector<float> kExpectedOutputRight = {
      00.0041040001f, 0.0083560003f, 0.012758000f, 0.017312000f, 0.022020001f,
      0.026883999f,   0.031906001f,  0.037087999f, 0.042431999f, 0.047940001f,
      0.053613998f,   0.059455998f,  0.065467998f, 0.071652003f, 0.078010000f,
      0.084543996f,   0.091256000f,  0.098148003f};
  const std::vector<std::vector<float>> kHrirDataLong =
      GenerateAudioData(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer + 1);

  AudioBuffer sh_hrirs(kHrirDataLong.size(), kHrirDataLong[0].size());
  sh_hrirs = kHrirDataLong;

  AudioBuffer input(kInputData.size(), kInputData[0].size());
  input = kInputData;

  AudioBuffer output(kNumStereoChannels, kInputData[0].size());
  output.Clear();

  FftManager fft_manager(kFramesPerBuffer);
  AmbisonicBinauralDecoder decoder(sh_hrirs, kFramesPerBuffer, &fft_manager);
  decoder.Process(input, &output);

  for (size_t sample = 0; sample < kFramesPerBuffer; ++sample) {
    EXPECT_NEAR(kExpectedOutputLeft[sample], output[0][sample], kEpsilonFloat);
    EXPECT_NEAR(kExpectedOutputRight[sample], output[1][sample], kEpsilonFloat);
  }
}

}  // namespace vraudio
