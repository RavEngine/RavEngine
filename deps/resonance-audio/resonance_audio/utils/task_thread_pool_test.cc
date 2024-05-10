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

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "base/integral_types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/logging.h"
#include "utils/task_thread_pool.h"

namespace vraudio {

namespace {

// The number of simultaneous worker threads to run for these tests.
const size_t kNumberOfThreads = 5;

// An extremely large number of worker threads to use when attempting to shut
// down the |TaskThreadPool| while it is still being initialized.
const size_t kHugeThreadCount = 64;

// An arbitrary numeric value to set when testing worker threads.
const int kModifiedValue = 113;

// A limited number of iterations to perform when testing worker threads which
// do continuous work over time.
const size_t kNumberOfIncrementOperations = 50;

// The number of times to repeat test loops in functions to insure reuse of
// worker threads.
const size_t kNumTestLoops = 3;

class TaskThreadPoolTest : public ::testing::Test {
 protected:
  TaskThreadPoolTest() {}
  ~TaskThreadPoolTest() override {}

  void SetUp() override { modified_values_.resize(kNumberOfThreads, 0); }

 public:
  // Helper worker task to asynchronously set values in worker threads.
  void ModifyValue(int* value_to_set) {
    std::lock_guard<std::mutex> worker_lock(modification_mutex_);
    EXPECT_NE(value_to_set, nullptr);
    *value_to_set = kModifiedValue;
  }

  // Helper worker task to asynchronously increment modified values over time
  // with enough interior delay to measure results.
  void IncrementValue(int* value_to_increment) {
    EXPECT_NE(value_to_increment, nullptr);
    for (size_t i = 0; i < kNumberOfIncrementOperations; ++i) {
      // Sleep briefly so this doesn't finish too quickly.
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      std::lock_guard<std::mutex> worker_lock(modification_mutex_);
      *value_to_increment += 1;
    }
  }

 protected:
  // Vector of numbers used for testing asynchronous threading results.
  std::vector<int> modified_values_;

  // A mutex for protecting modified_values_ from tsan (Thread Sanitizer)
  // failures.
  std::mutex modification_mutex_;
};

// This test verifies that TaskThreadPool actually executes tasks.
TEST_F(TaskThreadPoolTest, SetValuesInWorkerThreads) {
  TaskThreadPool thread_pool;
  EXPECT_TRUE(thread_pool.StartThreadPool(kNumberOfThreads));

  // Run this several times to insure that worker threads can be reused.
  for (size_t loop = 0; loop < kNumTestLoops; ++loop) {
    // Verify that all worker threads are available again.
    EXPECT_EQ(thread_pool.GetAvailableTaskThreadCount(), kNumberOfThreads);

    for (size_t i = 0; i < kNumberOfThreads; ++i) {
      modified_values_[i] = 0;
      const bool task_available = thread_pool.WaitUntilWorkerBecomesAvailable();
      EXPECT_TRUE(task_available);
      const bool task_submitted = thread_pool.RunOnWorkerThread(
          std::bind(&vraudio::TaskThreadPoolTest::ModifyValue, this,
                    &modified_values_[i]));
      EXPECT_TRUE(task_submitted);
    }

    // Wait until all threads become available again (trying to time their
    // completion seems to cause flakey tests).
    while (thread_pool.GetAvailableTaskThreadCount() < kNumberOfThreads) {
      // Wait briefly to allow tasks to execute.
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // Check results.
    for (size_t i = 0; i < kNumberOfThreads; ++i) {
      std::lock_guard<std::mutex> worker_lock(modification_mutex_);
      EXPECT_EQ(modified_values_[i], kModifiedValue);
    }
  }
}

// This test verifies that the |TaskThreadPool| cannot be shut down until all of
// its worker threads are brought to a ready state.
TEST_F(TaskThreadPoolTest, VerifyRapidShutdownWithLargeThreadCount) {
  TaskThreadPool thread_pool;
  EXPECT_TRUE(thread_pool.StartThreadPool(kHugeThreadCount));
}

// This test verifies the timeout features of assigning worker threads, as well
// as the continuous asynchronous operation of threads..
TEST_F(TaskThreadPoolTest, VerifyTimeoutsAndContinuousOperation) {
  // Preset |modified_values_| to known state.
  for (size_t i = 0; i < kNumberOfThreads; ++i) {
    modified_values_[i] = 0;
  }

  {
    TaskThreadPool thread_pool;
    EXPECT_TRUE(thread_pool.StartThreadPool(kNumberOfThreads));

    // Verify that all worker threads are available again.
    EXPECT_EQ(thread_pool.GetAvailableTaskThreadCount(), kNumberOfThreads);

    for (size_t i = 0; i < kNumberOfThreads; ++i) {
      modified_values_[i] = 0;
      const bool task_available = thread_pool.WaitUntilWorkerBecomesAvailable();
      EXPECT_TRUE(task_available);
      const bool task_submitted = thread_pool.RunOnWorkerThread(
          std::bind(&vraudio::TaskThreadPoolTest::IncrementValue, this,
                    &modified_values_[i]));
      EXPECT_TRUE(task_submitted);
    }

    // Verify that all worker threads are available again.
    EXPECT_EQ(thread_pool.GetAvailableTaskThreadCount(), 0U);

    // Trying to add one more task should fail.
    int extra_modified;
    EXPECT_FALSE(thread_pool.RunOnWorkerThread(std::bind(
        &vraudio::TaskThreadPoolTest::IncrementValue, this, &extra_modified)));

    // Wait briefly to allow tasks to execute.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verify that all of the tasks are still doing some work.
    for (size_t i = 0; i < kNumberOfThreads; ++i) {
      std::lock_guard<std::mutex> worker_lock(modification_mutex_);
      EXPECT_GT(modified_values_[i], 0);
    }
  }
  // To verify that all threads are shut down correctly, record the
  // |modified_values_|, wait briefly, and then make sure theyΩ have not
  // changed.
  std::vector<int> copy_of_modified_values = modified_values_;
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  for (size_t i = 0; i < kNumberOfThreads; ++i) {
    // NOTE:  modification_mutex_ is intentionally not used for this block so
    // that any race conditions might be caught if for some reason the worker
    // threads have not yet shut down.
    EXPECT_EQ(copy_of_modified_values[i], modified_values_[i]);
  }
}

}  // namespace

}  // namespace vraudio
