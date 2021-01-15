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

// Prevent Visual Studio from complaining about std::copy_n.
#if defined(_WIN32)
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "dsp/partitioned_fft_filter.h"

#include <cmath>
#include <iostream>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/logging.h"
#include "dsp/fft_manager.h"

namespace vraudio {

namespace {

// Permitted error in the output relative to the expected output. This value is
// 1e-3 as expected output is rounded for readability.
const float kFftEpsilon = 1e-3f;

// Length of input buffer for use in LongerShorterKernelTest.
const size_t kLength = 32;

// Helper function for use in LongerShorterKernelTest. Passes a dirac of length
// kLength through a filter followed by |zeros_iteration| zero vectors of the
// same length.
void ProcessFilterWithImpulseSignal(PartitionedFftFilter* filter,
                                    FftManager* fft_manager,
                                    size_t zeros_iteration,
                                    std::vector<float>* output_signal) {
  AudioBuffer signal_buffer(kNumMonoChannels, kLength);
  signal_buffer.Clear();
  signal_buffer[0][0] = 1.0f;
  AudioBuffer output_buffer(kNumMonoChannels, kLength);

  // Begin filtering with the original (small) kernel.
  PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(kNumMonoChannels,
                                                            kLength * 2);
  fft_manager->FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
  filter->Filter(freq_domain_buffer[0]);
  filter->GetFilteredSignal(&output_buffer[0]);
  output_signal->insert(output_signal->end(), output_buffer[0].begin(),
                        output_buffer[0].end());
  // Filter zeros to flush out internal buffers.
  std::fill(signal_buffer[0].begin(), signal_buffer[0].end(), 0.0f);
  for (size_t i = 0; i < zeros_iteration; ++i) {
    fft_manager->FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
    filter->Filter(freq_domain_buffer[0]);
    filter->GetFilteredSignal(&output_buffer[0]);
    output_signal->insert(output_signal->end(), output_buffer[0].begin(),
                          output_buffer[0].end());
  }
}

// Tests that convolution will work correctly when the input buffer length is
// not a power of two.
TEST(PartitionedFftFilterTest, NonPow2Test) {
  // Non power of two input buffer length.
  const size_t kMinNonPowTwo = 13;
  const size_t kMaxNonPowTwo = 41;
  for (size_t buffer_length = kMinNonPowTwo; buffer_length <= kMaxNonPowTwo;
       buffer_length += 2) {
    for (size_t filter_length = kLength; filter_length <= 3 * kLength;
         filter_length += kLength) {
      // Use a filter length that is a power of two.
      AudioBuffer kernel_buffer(kNumMonoChannels, filter_length);
      // Place to collect all of the output.
      std::vector<float> output_signal;
      // First set the kernel to a linear ramp.
      for (size_t i = 0; i < filter_length; ++i) {
        kernel_buffer[0][i] = static_cast<float>(i) / 4.0f;
      }
      FftManager fft_manager(buffer_length);
      PartitionedFftFilter filter(filter_length, buffer_length, &fft_manager);
      filter.SetTimeDomainKernel(kernel_buffer[0]);

      // Kronecker delta signal.
      AudioBuffer signal_buffer(kNumMonoChannels, buffer_length);
      signal_buffer.Clear();
      signal_buffer[0][0] = 1.0f;
      AudioBuffer output_buffer(kNumMonoChannels, buffer_length);
      // Create a freq domain buffer which should be fft_size
      // (i.e. NextPowTwo(buffer_length) * 2).
      PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(
          kNumMonoChannels, NextPowTwo(buffer_length) * 2);
      freq_domain_buffer.Clear();
      fft_manager.FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
      // Perform convolution.
      filter.Filter(freq_domain_buffer[0]);
      filter.GetFilteredSignal(&output_buffer[0]);
      output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                           output_buffer[0].end());
      while (output_signal.size() < kernel_buffer.num_frames()) {
        // Flush with zeros.
        freq_domain_buffer.Clear();
        output_buffer.Clear();
        filter.Filter(freq_domain_buffer[0]);
        filter.GetFilteredSignal(&output_buffer[0]);
        output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                             output_buffer[0].end());
      }
      // Ensure the output is identical to the input buffer. (large epsilon
      // needed due to large values in filter).
      for (size_t i = 0; i < kernel_buffer.num_frames(); ++i) {
        EXPECT_NEAR(kernel_buffer[0][i], output_signal[i],
                    kEpsilonFloat * 4.0f);
      }
    }
  }
}

// Tests that the outputs from the convolution are correct based on a
// precomputed vector from MATLAB.
TEST(PartitionedFftFilterTest, CorrectNonPowTwoOutputTest) {
  const size_t kBufferSize = 15;

  // Create an arbirary vector of size 16 for filter and 15 for input signal.
  const std::vector<float> kernel = {1.0f, 3.0f, 0.0f, 2.0f, 5.0f, 1.0f,
                                     3.0f, 2.0f, 0.0f, 4.0f, 1.0f, 3.0f,
                                     0.0f, 2.0f, 1.0f, 2.0f};
  const std::vector<float> signal = {2.0f, 3.0f, 3.0f, 4.0f, 0.0f,
                                     0.0f, 2.0f, 1.0f, 2.0f, 1.0f,
                                     3.0f, 2.0f, 4.0f, 0.0f, 2.0f};
  // Ideal output vector verified with MATLAB.
  const std::vector<float> ideal_output = {
      2.0f,  9.0f,  12.0f, 17.0f, 28.0f, 23.0f, 34.0f, 43.0f, 24.0f,
      37.0f, 40.0f, 43.0f, 57.0f, 49.0f, 50.0f, 57.0f, 65.0f, 57.0f,
      60.0f, 61.0f, 47.0f, 74.0f, 59.0f, 55.0f, 48.0f, 62.0f, 51.0f,
      69.0f, 51.0f, 54.0f, 55.0f, 56.0f, 45.0f, 43.0f, 33.0f, 24.0f,
      40.0f, 16.0f, 31.0f, 11.0f, 22.0f, 8.0f,  12.0f, 2.0f,  4.0f};

  AudioBuffer kernel_buffer(kNumMonoChannels, kernel.size());
  kernel_buffer[0] = kernel;

  AudioBuffer signal_buffer(kNumMonoChannels, signal.size());
  signal_buffer[0] = signal;

  AudioBuffer output_buffer(kNumMonoChannels, signal.size());

  std::vector<float> output_signal;
  output_signal.reserve(kernel.size() + signal.size() * 3);

  FftManager fft_manager(kBufferSize);
  PartitionedFftFilter filter(kernel.size(), kBufferSize, &fft_manager);
  filter.SetTimeDomainKernel(kernel_buffer[0]);

  PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(
      1, NextPowTwo(kBufferSize) * 2);
  fft_manager.FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
  filter.Filter(freq_domain_buffer[0]);
  filter.GetFilteredSignal(&output_buffer[0]);
  output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                       output_buffer[0].end());

  // Filter again with the same input.
  filter.Filter(freq_domain_buffer[0]);
  filter.GetFilteredSignal(&output_buffer[0]);
  output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                       output_buffer[0].end());

  // Filter zeros to flush out internal buffers.
  signal_buffer.Clear();
  fft_manager.FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
  filter.Filter(freq_domain_buffer[0]);
  filter.GetFilteredSignal(&output_buffer[0]);
  output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                       output_buffer[0].end());

  for (size_t sample = 0; sample < ideal_output.size(); ++sample) {
    EXPECT_NEAR(output_signal[sample], ideal_output[sample], kFftEpsilon);
  }
}

// Tests that we can switch to a time domain filter kernel of greater
// length than the original kernal provided at instantiation. It then tests that
// we can switch to a time domain filter kernel of lesser length than the
// original kernel.
TEST(PartitionedFftFilterTest, LongerShorterTimeDomainKernelTest) {
  AudioBuffer small_kernel_buffer(kNumMonoChannels, kLength);
  AudioBuffer big_kernel_buffer(kNumMonoChannels, kLength * 2);

  // Place to collect all of the output.
  std::vector<float> total_output_signal;

  // First set the kernels to linear ramps of differing lengths.
  for (size_t i = 0; i < kLength * 2; ++i) {
    if (i < kLength) {
      small_kernel_buffer[0][i] = static_cast<float>(i) / 4.0f;
    }
    big_kernel_buffer[0][i] = static_cast<float>(i) / 4.0f;
  }

  FftManager fft_manager(kLength);
  PartitionedFftFilter filter(small_kernel_buffer.num_frames(), kLength,
                              big_kernel_buffer.num_frames(), &fft_manager);

  filter.SetTimeDomainKernel(small_kernel_buffer[0]);
  ProcessFilterWithImpulseSignal(&filter, &fft_manager, 2,
                                 &total_output_signal);

  filter.SetTimeDomainKernel(big_kernel_buffer[0]);
  ProcessFilterWithImpulseSignal(&filter, &fft_manager, 3,
                                 &total_output_signal);

  filter.SetTimeDomainKernel(small_kernel_buffer[0]);
  ProcessFilterWithImpulseSignal(&filter, &fft_manager, 2,
                                 &total_output_signal);

  // Test to see if output from both kernels is present kLength * 2 zeros in
  // between.
  for (size_t i = 0; i < kLength; ++i) {
    EXPECT_NEAR(static_cast<float>(i) / 4.0f, total_output_signal[i],
                kFftEpsilon);
    EXPECT_NEAR(0.0f, total_output_signal[i + kLength], kFftEpsilon);
    EXPECT_NEAR(0.0f, total_output_signal[i + 2 * kLength], kFftEpsilon);
    EXPECT_NEAR(static_cast<float>(i) / 4.0f,
                total_output_signal[i + 3 * kLength], kFftEpsilon);
    EXPECT_NEAR(static_cast<float>(i + kLength) / 4.0f,
                total_output_signal[i + 4 * kLength], kFftEpsilon);
    EXPECT_NEAR(0.0f, total_output_signal[i + 5 * kLength], kFftEpsilon);
    EXPECT_NEAR(0.0f, total_output_signal[i + 6 * kLength], kFftEpsilon);
    EXPECT_NEAR(static_cast<float>(i) / 4.0f,
                total_output_signal[i + 7 * kLength], kFftEpsilon);
    EXPECT_NEAR(0.0f, total_output_signal[i + 8 * kLength], kFftEpsilon);
    EXPECT_NEAR(0.0f, total_output_signal[i + 9 * kLength], kFftEpsilon);
  }
}

// Tests that the outputs from the same convolution performed with different FFT
// sizes will all return equal results, including when the FFT size is equal to
// the kernel size.
TEST(PartitionedFftFilterTest, PartitionSizeInvarianceTest) {
  const std::vector<size_t> kFftSizes = {32, 64, 128};

  // Create an arbirary vector of size 128 for both filter and input signal.
  const size_t max_fft_size = kFftSizes[kFftSizes.size() - 1];
  AudioBuffer kernel_buffer(kNumMonoChannels, max_fft_size);
  AudioBuffer signal_buffer(kNumMonoChannels, max_fft_size);
  for (size_t i = 0; i < max_fft_size; ++i) {
    kernel_buffer[0][i] = static_cast<float>(static_cast<int>(i) % 13 - 7);
    signal_buffer[0][i] = static_cast<float>(static_cast<int>(i) % 17 - 9);
  }

  std::vector<std::vector<float>> output_signal(
      kFftSizes.size(), std::vector<float>(max_fft_size * 2));

  // Iterate over 3 fft sizes.
  for (size_t fft_idx = 0; fft_idx < kFftSizes.size(); ++fft_idx) {
    const size_t chunk_size = kFftSizes[fft_idx] / 2;
    FftManager fft_manager(chunk_size);
    PartitionedFftFilter filter(max_fft_size, chunk_size, &fft_manager);
    filter.SetTimeDomainKernel(kernel_buffer[0]);

    AudioBuffer input_chunk(kNumMonoChannels, chunk_size);
    AudioBuffer output_chunk(kNumMonoChannels, chunk_size);
    PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(
        1, kFftSizes[fft_idx]);

    // Break the input signal into chunks of fft size / 2.
    for (size_t chunk = 0; chunk < (max_fft_size / chunk_size); ++chunk) {
      AudioBuffer signal_block(kNumMonoChannels, chunk_size);
      std::copy_n(signal_buffer[0].begin() + (chunk * chunk_size), chunk_size,
                  signal_block[0].begin());

      fft_manager.FreqFromTimeDomain(signal_block[0], &freq_domain_buffer[0]);
      filter.Filter(freq_domain_buffer[0]);
      filter.GetFilteredSignal(&output_chunk[0]);
      output_signal[fft_idx].insert(output_signal[fft_idx].end(),
                                    output_chunk[0].begin(),
                                    output_chunk[0].end());
    }
  }
  // Now test the outputs are pretty much equal to one another (to the first).
  for (size_t i = 1; i < kFftSizes.size(); ++i) {
    for (size_t sample = 0; sample < output_signal[0].size(); ++sample) {
      EXPECT_NEAR(output_signal[0][sample], output_signal[i][sample],
                  kFftEpsilon);
    }
  }
}

// Tests that the outputs from the convolution are correct based on a
// precomputed vector from MATLAB.
TEST(PartitionedFftFilterTest, CorrectOutputTest) {
  const size_t kBufferSize = 32;

  // Create an arbirary vector of size 32 for both filter and input signal.
  const std::vector<float> kernel = {
      1.0f, 3.0f, 0.0f, 2.0f, 5.0f, 1.0f, 3.0f, 2.0f, 0.0f, 4.0f, 1.0f,
      3.0f, 0.0f, 2.0f, 1.0f, 2.0f, 2.0f, 1.0f, 0.0f, 3.0f, 5.0f, 2.0f,
      3.0f, 0.0f, 1.0f, 4.0f, 2.0f, 0.0f, 1.0f, 0.0f, 2.0f, 1.0f};
  const std::vector<float> signal = {
      2.0f, 1.0f, 3.0f, 3.0f, 2.0f, 4.0f, 2.0f, 1.0f, 3.0f, 4.0f, 5.0f,
      3.0f, 2.0f, 2.0f, 5.0f, 4.0f, 5.0f, 3.0f, 3.0f, 4.0f, 0.0f, 0.0f,
      2.0f, 1.0f, 2.0f, 1.0f, 3.0f, 2.0f, 4.0f, 0.0f, 2.0f, 1.0f};

  AudioBuffer kernel_buffer(kNumMonoChannels, kernel.size());
  kernel_buffer[0] = kernel;

  AudioBuffer signal_buffer(kNumMonoChannels, signal.size());
  signal_buffer[0] = signal;

  AudioBuffer output_buffer(kNumMonoChannels, kernel.size());

  std::vector<float> output_signal;
  output_signal.reserve(kernel.size() + signal.size());

  FftManager fft_manager(kBufferSize);
  PartitionedFftFilter filter(kernel.size(), kBufferSize, &fft_manager);
  filter.SetTimeDomainKernel(kernel_buffer[0]);

  PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(kNumMonoChannels,
                                                            kBufferSize * 2);
  fft_manager.FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
  filter.Filter(freq_domain_buffer[0]);
  filter.GetFilteredSignal(&output_buffer[0]);
  output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                       output_buffer[0].end());

  // Filter zeros to flush out internal buffers.
  signal_buffer.Clear();
  fft_manager.FreqFromTimeDomain(signal_buffer[0], &freq_domain_buffer[0]);
  filter.Filter(freq_domain_buffer[0]);
  filter.GetFilteredSignal(&output_buffer[0]);
  output_signal.insert(output_signal.end(), output_buffer[0].begin(),
                       output_buffer[0].end());

  // Ideal output vector verified with MATLAB.
  const std::vector<float> kIdeal = {
      2.0f,   7.0f,   6.0f,   16.0f,  23.0f,  23.0f,  42.0f,  36.0f,  38.0f,
      62.0f,  51.0f,  66.0f,  67.0f,  72.0f,  88.0f,  90.0f,  90.0f,  95.0f,
      104.0f, 118.0f, 127.0f, 113.0f, 123.0f, 131.0f, 116.0f, 144.0f, 119.0f,
      126.0f, 138.0f, 138.0f, 153.0f, 142.0f, 130.0f, 125.0f, 137.0f, 142.0f,
      121.0f, 113.0f, 99.0f,  112.0f, 84.0f,  89.0f,  68.0f,  66.0f,  74.0f,
      54.0f,  54.0f,  59.0f,  53.0f,  42.0f,  42.0f,  26.0f,  32.0f,  28.0f,
      18.0f,  15.0f,  19.0f,  9.0f,   12.0f,  5.0f,   4.0f,   4.0f,   1.0f};
  for (size_t sample = 0; sample < kIdeal.size(); ++sample) {
    EXPECT_NEAR(output_signal[sample], kIdeal[sample], kFftEpsilon);
  }
}

// Tests that the outputs from the convolution are equal to zero when the inputs
// are all zero.
TEST(PartitionedFftFilterTest, ZeroInputZeroOutputTest) {
  const size_t kChunkSize = 16;

  // Create an arbirary vector of size 16 for filter.
  const std::vector<float> kernel = {1.0f, 3.0f, 0.0f, 2.0f, 5.0f, 1.0f,
                                     3.0f, 2.0f, 0.0f, 4.0f, 1.0f, 3.0f,
                                     0.0f, 2.0f, 1.0f, 2.0f};

  AudioBuffer kernel_buffer(kNumMonoChannels, kernel.size());
  kernel_buffer[0] = kernel;

  std::vector<float> output_signal;
  output_signal.reserve(kernel.size() * 2);

  FftManager fft_manager(kChunkSize);
  PartitionedFftFilter filter(kernel_buffer[0].size(), kChunkSize,
                              &fft_manager);
  filter.SetTimeDomainKernel(kernel_buffer[0]);

  AudioBuffer output_chunk(kNumMonoChannels, kChunkSize);

  // Filter zeros to flush out internal buffers.
  AudioBuffer zero_signal(kNumMonoChannels, kChunkSize);
  zero_signal.Clear();

  PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(kNumMonoChannels,
                                                            kChunkSize * 2);
  for (size_t j = 0; j < 2 * kernel.size() / kChunkSize; ++j) {
    fft_manager.FreqFromTimeDomain(zero_signal[0], &freq_domain_buffer[0]);
    filter.Filter(freq_domain_buffer[0]);
    filter.GetFilteredSignal(&output_chunk[0]);
    output_signal.insert(output_signal.end(), output_chunk[0].begin(),
                         output_chunk[0].end());
  }

  // Check that all output samples are practically 0.0.
  for (size_t sample = 0; sample < output_signal.size(); ++sample) {
    EXPECT_EQ(output_signal[sample], 0.0f);
  }
}

// This test uses a simple shifted dirac impulse as kernel and checks the
// resulting signal delay.
TEST(PartitionedFftFilterTest, DiracImpulseTest) {
  const size_t kFilterSize = 32;
  const size_t kNumBlocks = 4;
  const size_t kSignalSize = kFilterSize * kNumBlocks;

  // Generate a saw tooth signal.
  AudioBuffer test_signal(kNumMonoChannels, kSignalSize);
  for (size_t i = 0; i < kSignalSize; ++i) {
    test_signal[0][i] = static_cast<float>(i % 5);
  }

  FftManager fft_manager(kFilterSize);
  PartitionedFftFilter fft_filter(kFilterSize, kFilterSize, &fft_manager);

  AudioBuffer kernel(kNumMonoChannels, kFilterSize);
  // Construct dirac impulse response. This kernel should result in a delay of
  // length "|kFilterSize| / 2".
  kernel.Clear();
  kernel[0][kFilterSize / 2] = 1.0f;
  fft_filter.SetTimeDomainKernel(kernel[0]);

  std::vector<float> filtered_signal;
  filtered_signal.reserve(kSignalSize);

  AudioBuffer filtered_block(kNumMonoChannels, kFilterSize);
  PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(kNumMonoChannels,
                                                            kFilterSize * 2);

  for (size_t b = 0; b < kNumBlocks; ++b) {
    AudioBuffer signal_block(kNumMonoChannels, kFilterSize);
    std::copy_n(test_signal[0].begin() + b * kFilterSize, kFilterSize,
                signal_block[0].begin());

    fft_manager.FreqFromTimeDomain(signal_block[0], &freq_domain_buffer[0]);
    fft_filter.Filter(freq_domain_buffer[0]);
    fft_filter.GetFilteredSignal(&filtered_block[0]);
    filtered_signal.insert(filtered_signal.end(), filtered_block[0].begin(),
                           filtered_block[0].end());
  }

  for (size_t i = 0; i < kFilterSize / 2; ++i) {
    // First "filter_size / 2" samples should be zero padded due to the applied
    // delay.
    EXPECT_NEAR(filtered_signal[i], 0.0f, 1e-5f);
  }
  for (size_t i = kFilterSize / 2; i < kSignalSize; ++i) {
    // Test if signal is delayed by exactly |kFilterSize| / 2 samples.
    EXPECT_NEAR(filtered_signal[i], test_signal[0][i - kFilterSize / 2], 1e-5f);
  }
}

}  // namespace

class PartitionedFftFilterFrequencyBufferTest : public ::testing::Test {
 protected:
  PartitionedFftFilterFrequencyBufferTest() {}
  // Virtual methods from ::testing::Test
  ~PartitionedFftFilterFrequencyBufferTest() override {}
  void SetUp() override {}
  void TearDown() override {}

  void SetFreqDomainBuffer(PartitionedFftFilter* filter,
                           FftManager* fft_manager) {
    std::vector<std::vector<float>> values_vectors(
        filter->num_partitions_,
        std::vector<float>(fft_manager->GetFftSize(), 0.0f));
    for (size_t i = 0; i < filter->num_partitions_; ++i) {
      values_vectors[i][0] = static_cast<float>(i + 1);
      filter->freq_domain_buffer_[i] = values_vectors[i];
    }
  }

  void SetFreqDomainKernel(PartitionedFftFilter* filter) {
    // Put a Kronecker delta in the first partition, a kronecker delta times 2
    // in the second, a kronecker delta times 3 in the third, etc...
    AudioBuffer kernel(kNumMonoChannels, filter->filter_size_);
    for (size_t i = 0; i < filter->num_partitions_; ++i) {
      for (size_t sample = 0; sample < filter->chunk_size_; ++sample) {
        kernel[0][i * filter->chunk_size_ + sample] = static_cast<float>(i + 1);
      }
    }
    filter->SetTimeDomainKernel(kernel[0]);
  }

  void TestFrequencyBufferReset(size_t initial_size, size_t bigger_size,
                                size_t smaller_size,
                                PartitionedFftFilter* filter,
                                FftManager* fft_manager) {
    const size_t initial_num_partitions = 2 * initial_size;
    const size_t bigger_num_partitions = 2 * bigger_size;
    const size_t smaller_num_partitions = 2 * smaller_size;
    // Set the |freq_domain_buffer_| inside |filter| such that it has known
    // values.
    SetFreqDomainBuffer(filter, fft_manager);
    AudioBuffer initial_freq_domain_buffer;
    initial_freq_domain_buffer = filter->freq_domain_buffer_;

    // Set the current front buffer to something other than zero and less than
    // the number of partitions. If |num_partitions_| is 1 it is set to 0.
    filter->curr_front_buffer_ = filter->num_partitions_ / 2;

    // Reset the |freq_domain_buffers_| for a new bigger kernel size.
    filter->ResetFreqDomainBuffers(bigger_size * fft_manager->GetFftSize());
    AudioBuffer bigger_freq_domain_buffer;
    bigger_freq_domain_buffer = filter->freq_domain_buffer_;
    // Verify that the input index has been reset.
    EXPECT_EQ(filter->curr_front_buffer_, 0U);

    // Set the current front buffer to something other than zero and less than
    // the number of partitions. If |num_partitions_| is 1 it is set to 0.
    filter->curr_front_buffer_ = filter->num_partitions_ / 2;

    // Reset the |freq_domain_buffers_| for a new smaller kernel size.
    filter->ResetFreqDomainBuffers(smaller_size * fft_manager->GetFftSize());
    AudioBuffer smaller_freq_domain_buffer;
    smaller_freq_domain_buffer = filter->freq_domain_buffer_;
    // Verify that the input index has been reset.
    EXPECT_EQ(filter->curr_front_buffer_, 0U);

    // Expect the following to have happened:
    // Initially there are 16 partitions with the first element in each channel
    // of the |freq_domain_buffers_| in |filter| being 1, 2, ..., 16.
    for (size_t i = 0; i < initial_num_partitions; ++i) {
      EXPECT_EQ(initial_freq_domain_buffer[i][0], static_cast<float>(i + 1));
    }

    // |curr_front_buffer_| is set to 8. After a reset with a larger kernel we
    // expect all of the buffers to have been copied into the new
    // |freq_domain_buffers_| starting from |curr_front_buffer_| and wrapping
    // round with the remaining channels set to zero.
    for (size_t i = 0; i < initial_num_partitions; ++i) {
      EXPECT_EQ(bigger_freq_domain_buffer[i][0],
                initial_freq_domain_buffer[(initial_size + i) %
                                           initial_num_partitions][0]);
    }
    for (size_t i = initial_num_partitions; i < bigger_num_partitions; ++i) {
      EXPECT_EQ(bigger_freq_domain_buffer[i][0], 0.0f);
    }

    // |curr_front_buffer_| is then set to 5. After a reset with a smaller
    // kernel ammounting to 10 partitions we expect just the first 10 buffers to
    // have been copied into the new |freq_domain_buffers_| starting from
    // |curr_front_buffer_| and wrapping round.
    for (size_t i = 0; i < smaller_num_partitions; ++i) {
      EXPECT_EQ(smaller_freq_domain_buffer[i][0],
                bigger_freq_domain_buffer[(bigger_size + i) %
                                          bigger_num_partitions][0]);
    }
  }

  void TestLongerShorterFrequencyDomainKernel(size_t buffer_size) {
    AudioBuffer small_kernel_buffer(kNumMonoChannels, buffer_size);
    AudioBuffer big_kernel_buffer(kNumMonoChannels, 2 * buffer_size);

    // Place to collect all of the output.
    std::vector<float> total_output_signal;

    // First set the kernels to linear ramps of differing lengths.
    for (size_t i = 0; i < 2 * buffer_size; ++i) {
      if (i < buffer_size) {
        small_kernel_buffer[0][i] = static_cast<float>(i) / 4.0f;
      }
      big_kernel_buffer[0][i] = static_cast<float>(i) / 4.0f;
    }

    FftManager fft_manager(buffer_size);
    PartitionedFftFilter filter(buffer_size, buffer_size, 2 * buffer_size,
                                &fft_manager);

    // Generate the small and large frequency domain buffers.
    filter.SetTimeDomainKernel(small_kernel_buffer[0]);
    const size_t small_num_partitions = filter.num_partitions_;
    PartitionedFftFilter::FreqDomainBuffer small_freq_domain_kernel =
        PartitionedFftFilter::FreqDomainBuffer(small_num_partitions,
                                               fft_manager.GetFftSize());
    for (size_t i = 0; i < small_num_partitions; ++i) {
      small_freq_domain_kernel[i] = filter.kernel_freq_domain_buffer_[i];
    }

    filter.SetTimeDomainKernel(big_kernel_buffer[0]);
    const size_t big_num_partitions = filter.num_partitions_;
    PartitionedFftFilter::FreqDomainBuffer big_freq_domain_kernel =
        PartitionedFftFilter::FreqDomainBuffer(big_num_partitions,
                                               fft_manager.GetFftSize());
    for (size_t i = 0; i < big_num_partitions; ++i) {
      big_freq_domain_kernel[i] = filter.kernel_freq_domain_buffer_[i];
    }

    filter.SetFreqDomainKernel(small_freq_domain_kernel);
    ProcessFilterWithImpulseSignal(&filter, &fft_manager, 2,
                                   &total_output_signal);

    filter.SetFreqDomainKernel(big_freq_domain_kernel);
    ProcessFilterWithImpulseSignal(&filter, &fft_manager, 3,
                                   &total_output_signal);

    filter.SetFreqDomainKernel(small_freq_domain_kernel);
    ProcessFilterWithImpulseSignal(&filter, &fft_manager, 2,
                                   &total_output_signal);

    // Test to see if output from both kernels is present |big_size| zeros in
    // between.
    for (size_t i = 0; i < buffer_size; ++i) {
      EXPECT_NEAR(static_cast<float>(i) / 4.0f, total_output_signal[i],
                  kFftEpsilon);
      EXPECT_NEAR(0.0f, total_output_signal[i + buffer_size], kFftEpsilon);
      EXPECT_NEAR(0.0f, total_output_signal[i + 2 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(static_cast<float>(i) / 4.0f,
                  total_output_signal[i + 3 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(static_cast<float>(i + buffer_size) / 4.0f,
                  total_output_signal[i + 4 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(0.0f, total_output_signal[i + 5 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(0.0f, total_output_signal[i + 6 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(static_cast<float>(i) / 4.0f,
                  total_output_signal[i + 7 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(0.0f, total_output_signal[i + 8 * buffer_size], kFftEpsilon);
      EXPECT_NEAR(0.0f, total_output_signal[i + 9 * buffer_size], kFftEpsilon);
    }
  }

  void TestPartitionReplacement(PartitionedFftFilter* filter,
                                FftManager* fft_manager) {
    // Fill the first partition with a kronecker delta in the first partition, a
    // kronecker delta times 2 in the second, a kronecker delta times 3 in the
    // third, etc...
    SetFreqDomainKernel(filter);
    // Get a copy of the kernel.
    AudioBuffer initial_kernel;
    initial_kernel = filter->kernel_freq_domain_buffer_;

    // Create a freq domain kernel chunk with the same values as the final
    // partition.
    AudioBuffer kernel_chunk(kNumMonoChannels, filter->chunk_size_);
    kernel_chunk.Clear();
    AudioBuffer::Channel& kernel_chunk_channel = kernel_chunk[0];
    for (size_t sample = 0; sample < filter->chunk_size_; ++sample) {
      kernel_chunk_channel[sample] =
          static_cast<float>(filter->num_partitions_);
    }
    // The partition we will replace.
    const size_t kReplacePartitionIndex = 2;
    // Replace a partition with a new time domain kernel chunk (all zeros).
    filter->ReplacePartition(kReplacePartitionIndex, kernel_chunk_channel);

    // Get a copy of the kernel after replacing the 3rd freq domain partition.
    AudioBuffer replaced_kernel;
    replaced_kernel = filter->kernel_freq_domain_buffer_;

    for (size_t i = 0; i < filter->num_partitions_; ++i) {
      for (size_t sample = 0; sample < fft_manager->GetFftSize(); ++sample) {
        // Expect that all of the other partitions are unchanged.
        if (i == kReplacePartitionIndex) {
          // Check if the chosen partition has been altered.
          EXPECT_EQ(initial_kernel[filter->num_partitions_ - 1][sample],
                    replaced_kernel[i][sample]);
        } else {
          // Check if all of the other partitions are unchanged.
          EXPECT_EQ(initial_kernel[i][sample], replaced_kernel[i][sample]);
        }
      }
    }
  }

  void TestFilterLengthSetter(PartitionedFftFilter* filter,
                              FftManager* fft_manager) {
    // Set the |freq_domain_buffer_| inside |filter| such that it has known
    // values (In the time domain, all value 1, then 2 etc.. per partition).
    SetFreqDomainKernel(filter);

    // Get a copy of the kernel and number of partitions before we set the
    // filter length.
    AudioBuffer initial_kernel;
    initial_kernel = filter->kernel_freq_domain_buffer_;
    const size_t initial_num_partitions = filter->num_partitions_;

    filter->SetFilterLength(filter->filter_size_ / 2);

    // Get a copy after we half the filter length.
    AudioBuffer half_kernel;
    half_kernel = filter->kernel_freq_domain_buffer_;
    const size_t half_num_partitions = filter->num_partitions_;
    EXPECT_EQ(initial_num_partitions, 2 * half_num_partitions);

    filter->SetFilterLength(filter->filter_size_ * 2);

    // Get a copy after we re-double the filter length.
    AudioBuffer redouble_kernel;
    redouble_kernel = filter->kernel_freq_domain_buffer_;
    EXPECT_EQ(initial_num_partitions, filter->num_partitions_);

    for (size_t i = 0; i < half_num_partitions; ++i) {
      for (size_t sample = 0; sample < fft_manager->GetFftSize(); ++sample) {
        // Check that the first two partitions in all 3 cases are the same.
        EXPECT_EQ(initial_kernel[i][sample], half_kernel[i][sample]);
        EXPECT_EQ(initial_kernel[i][sample], redouble_kernel[i][sample]);
        // Check that the final two partitions, after resizing the filters
        // frequency domain buffers, contain only zeros.
        EXPECT_EQ(redouble_kernel[i + half_num_partitions][sample], 0.0f);
      }
    }
  }
};

// Tests whether the reordering of the channels of |freq_domain_buffers_| is as
// expected after calling |ResetFreqDomainBuffers| with both longer and shorter
// filters.
TEST_F(PartitionedFftFilterFrequencyBufferTest, FrequencyBufferResetTest) {
  const size_t kBufferSize = 32;
  const size_t kInitialFilterSizeFactor = 8;
  const size_t kBiggerFilterSizeFactor = 10;
  const size_t kSmallerFilterSizeFactor = 5;
  // Initially there will be 16 partitions, then 20, then 10.
  FftManager fft_manager(kBufferSize);
  PartitionedFftFilter filter(
      kInitialFilterSizeFactor * kBufferSize * 2, kBufferSize,
      kBiggerFilterSizeFactor * kBufferSize * 2, &fft_manager);
  TestFrequencyBufferReset(kInitialFilterSizeFactor, kBiggerFilterSizeFactor,
                           kSmallerFilterSizeFactor, &filter, &fft_manager);
}

// Tests that we can switch to a frequency domain filter kernel with a greater
// number of partitions than the original kernel provided at instantiation.
// Ensures that we can switch to a frequency domain filter kernel with less
// partitions than the original kernel.
TEST_F(PartitionedFftFilterFrequencyBufferTest,
       LongerShorterFrequencyDomainKernelTest) {
  TestLongerShorterFrequencyDomainKernel(kLength);
}

// Tests whether replacing an individual partition works correctly.
TEST_F(PartitionedFftFilterFrequencyBufferTest, ReplacePartitionTest) {
  const size_t kBufferSize = 32;
  const size_t kFilterSizeFactor = 5;
  // A filter with 5 partitions.
  FftManager fft_manager(kBufferSize);
  PartitionedFftFilter filter(kFilterSizeFactor * kBufferSize * 2, kBufferSize,
                              &fft_manager);
  TestPartitionReplacement(&filter, &fft_manager);
}

// Tests whether setting the length of the filter kernel, and thus the number
// of partitions gives the expected results.
TEST_F(PartitionedFftFilterFrequencyBufferTest, SetFilterLengthTest) {
  const size_t kBufferSize = 32;
  const size_t kFilterSizeFactor = 5;
  // A filter with 5 partitions.
  FftManager fft_manager(kBufferSize);
  PartitionedFftFilter filter(kFilterSizeFactor * kBufferSize * 2, kBufferSize,
                              kFilterSizeFactor * kBufferSize * 4,
                              &fft_manager);
  TestFilterLengthSetter(&filter, &fft_manager);
}

}  // namespace vraudio
