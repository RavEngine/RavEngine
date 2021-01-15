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

#include "geometrical_acoustics/parallel_for.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace vraudio {

namespace {

// Tests multi-threaded increment of a single variable, using increasing numbers
// of threads.
TEST(ParallelForTest, IncreasingThreadCounts) {
  const size_t kNumIterations = 1000;
  for (unsigned int num_threads = 1; num_threads < 16; ++num_threads) {
    std::mutex mutex;
    std::condition_variable condition_variable;
    size_t index = 0;
    ParallelFor(num_threads, kNumIterations, [&](const size_t i) {
      std::unique_lock<std::mutex> lock(mutex);
      while (i != index) {
        condition_variable.wait(lock);
      }
      ++index;
      condition_variable.notify_all();
    });
    EXPECT_EQ(kNumIterations, index);
  }
}

// Tests recursive use of ParallelFor().
TEST(ParallelForTest, RecursiveParallelFor) {
  std::atomic<int> counter(0);
  ParallelFor(16U, 16, [&](const size_t i) {
    ParallelFor(16U, 16, [&](const size_t j) {
      for (int k = 0; k < 16; ++k) {
        ++counter;
      }
    });
  });
  EXPECT_EQ(16 * 16 * 16, counter.load());
}

}  // namespace

}  // namespace vraudio
