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

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "base/logging.h"
#include "utils/task_thread_pool.h"

namespace vraudio {

void ParallelFor(unsigned int num_threads, size_t num_iterations,
                 const std::function<void(const size_t)>& function) {
  TaskThreadPool worker_thread_pool;
  CHECK(worker_thread_pool.StartThreadPool(num_threads));

  for (size_t i = 0; i < num_iterations; ++i) {
    while (!worker_thread_pool.WaitUntilWorkerBecomesAvailable()) {
    }

    const TaskThreadPool::TaskClosure task = [i, &function]() { function(i); };
    CHECK(worker_thread_pool.RunOnWorkerThread(task));
  }
  worker_thread_pool.StopThreadPool();
}

}  // namespace vraudio
