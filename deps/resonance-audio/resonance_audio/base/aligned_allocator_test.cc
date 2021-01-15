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

#include "base/aligned_allocator.h"

#include <cstddef>
#include <vector>

#include "base/integral_types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/simd_utils.h"

using vraudio::AlignedAllocator;

namespace {

// Helper method to test memory alignment.
template <size_t Alignment>
void TestAlignedAllocator() {
  static const size_t kRuns = 1000;
  for (size_t run = 0; run < kRuns; ++run) {
    std::vector<float, AlignedAllocator<float, Alignment> > aligned_vector(1);
    const bool is_aligned =
        ((reinterpret_cast<size_t>(&aligned_vector[0]) & (Alignment - 1)) == 0);
    EXPECT_TRUE(is_aligned);
  }
}

}  // namespace

// Allocates multiple std::vectors using the AlignedAllocator and tests if the
// allocated memory is aligned.
TEST(AlignedAlocatorTest, TestAlignment) {
  TestAlignedAllocator<2>();
  TestAlignedAllocator<4>();
  TestAlignedAllocator<16>();
  TestAlignedAllocator<32>();
  TestAlignedAllocator<64>();
}

