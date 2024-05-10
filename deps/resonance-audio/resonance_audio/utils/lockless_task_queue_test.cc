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

#include "utils/lockless_task_queue.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/logging.h"

namespace vraudio {

namespace {
// Number of task producer threads.
static const size_t kNumTaskProducers = 5;

// Atomic thread counter used to trigger a simultaneous execution of all
// threads.
static std::atomic<size_t> s_thread_count(0);

// Waits until all threads are initialized.
static void WaitForProducerThreads() {
  static std::mutex mutex;
  static std::condition_variable cond_var;
  std::unique_lock<std::mutex> lock(mutex);

  if (++s_thread_count < kNumTaskProducers) {
    cond_var.wait(lock);
  } else {
    cond_var.notify_all();
  }
}

static void IncVectorAtIndex(std::vector<size_t>* work_vector_ptr,
                             size_t index) {
  ++(*work_vector_ptr)[index];
}

class TaskProducer {
 public:
  TaskProducer(LocklessTaskQueue* queue, std::vector<size_t>* work_vector_ptr,
               int delay_ms)
      : producer_thread_(new std::thread(std::bind(
            &TaskProducer::Produce, this, queue, work_vector_ptr, delay_ms))) {
  }

  TaskProducer(TaskProducer&& task)
      : producer_thread_(std::move(task.producer_thread_)) {}

  void Join() {
    if (producer_thread_->joinable()) {
      producer_thread_->join();
    }
  }

 private:
  void Produce(LocklessTaskQueue* queue, std::vector<size_t>* work_vector_ptr,
               int delay_ms) {
    WaitForProducerThreads();

    for (size_t i = 0; i < work_vector_ptr->size(); ++i) {
      queue->Post(std::bind(IncVectorAtIndex, work_vector_ptr, i));
      if (delay_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      }
    }
  }
  std::unique_ptr<std::thread> producer_thread_;
};

}  // namespace

class LocklessTaskQueueTest : public ::testing::Test {
 protected:
  // Virtual methods from ::testing::Test
  ~LocklessTaskQueueTest() override {}
  void SetUp() override {}
  void TearDown() override {}

  // Helper method to initialize and run the concurrency test with multiple
  // task producers and a single task executor.
  void ConcurrentThreadsMultipleTaskProducerSingleTaskExecutorTest(
      int producer_delay_ms) {
    s_thread_count = 0;
    const size_t kTasksPerProducer = 50;
    work_vector_.resize(kNumTaskProducers);
    std::fill(work_vector_.begin(), work_vector_.end(), 0);

    LocklessTaskQueue task_queue(kNumTaskProducers * kTasksPerProducer);

    std::vector<TaskProducer> task_producer_tasks;
    for (size_t i = 0; i < kNumTaskProducers; ++i) {
      task_producer_tasks.emplace_back(&task_queue, &work_vector_,
                                       producer_delay_ms);
    }
    WaitForProducerThreads();
    task_queue.Execute();

    for (auto& producer : task_producer_tasks) {
      producer.Join();
    }
    task_queue.Execute();

    for (size_t worker_count : work_vector_) {
      EXPECT_EQ(worker_count, kNumTaskProducers);
    }
  }

  std::vector<size_t> work_vector_;
};

TEST_F(LocklessTaskQueueTest, MaxTasks) {
  LocklessTaskQueue task_queue(1);

  work_vector_.resize(1, 0);

  task_queue.Execute();

  task_queue.Post(std::bind(IncVectorAtIndex, &work_vector_, 0));
  // Second task should be dropped since queue is initialized with size 1.
  task_queue.Post(std::bind(IncVectorAtIndex, &work_vector_, 0));
  task_queue.Execute();

  EXPECT_EQ(work_vector_[0], 1U);

  task_queue.Post(std::bind(IncVectorAtIndex, &work_vector_, 0));
  // Second task should be dropped since queue is initialized with size 1.
  task_queue.Post(std::bind(IncVectorAtIndex, &work_vector_, 0));
  task_queue.Execute();

  EXPECT_EQ(work_vector_[0], 2U);
}

TEST_F(LocklessTaskQueueTest, Clear) {
  LocklessTaskQueue task_queue(1);

  work_vector_.resize(1, 0);

  task_queue.Post(std::bind(IncVectorAtIndex, &work_vector_, 0));
  task_queue.Clear();
  task_queue.Execute();

  EXPECT_EQ(work_vector_[0], 0U);
}

TEST_F(LocklessTaskQueueTest, SynchronousTaskExecution) {
  const size_t kNumRounds = 5;
  const size_t kNumTasksPerRound = 20;

  LocklessTaskQueue task_queue(kNumTasksPerRound);

  work_vector_.resize(kNumTasksPerRound, 0);

  for (size_t r = 0; r < kNumRounds; ++r) {
    for (size_t t = 0; t < kNumTasksPerRound; ++t) {
      task_queue.Post(std::bind(IncVectorAtIndex, &work_vector_, t));
    }
    task_queue.Execute();
  }

  for (size_t t = 0; t < kNumTasksPerRound; ++t) {
    EXPECT_EQ(work_vector_[t], kNumRounds);
  }
}

TEST_F(LocklessTaskQueueTest, SynchronousInOrderTaskExecution) {
  const size_t kNumTasksPerRound = 20;

  LocklessTaskQueue task_queue(kNumTasksPerRound);

  work_vector_.resize(kNumTasksPerRound, 0);
  work_vector_[0] = 1;

  const auto accumulate_from_lower_index_task = [this](size_t index) {
    work_vector_[index] += work_vector_[index - 1];
  };
  for (size_t t = 1; t < kNumTasksPerRound; ++t) {
    task_queue.Post(std::bind(accumulate_from_lower_index_task, t));
  }
  task_queue.Execute();

  for (size_t t = 0; t < kNumTasksPerRound; ++t) {
    EXPECT_EQ(work_vector_[t], 1U);
  }
}

// Tests concurrency of multiple producers and a single executor.
TEST_F(LocklessTaskQueueTest,
       ConcurrentThreadsMultipleFastProducersSingleExecutor) {
  // Test fast producers and a fast consumer.
  ConcurrentThreadsMultipleTaskProducerSingleTaskExecutorTest(
      0 /* producer delay in ms */);
  ConcurrentThreadsMultipleTaskProducerSingleTaskExecutorTest(
      1 /* producer delay in ms */);
}

}  // namespace vraudio
