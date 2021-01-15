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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PARALLEL_FOR_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PARALLEL_FOR_H_

#include <algorithm>
#include <functional>
#include <thread>

namespace vraudio {

// Gets the number of hardware threads; always returns more than zero.
//
// @return Number of hardware threads.
inline unsigned int GetNumberOfHardwareThreads() {
  // According to the standard, hardware_concurrency() may return zero if "this
  // value is not computable or well defined". In that case, we want to have a
  // thread count of one (instead of zero).
  return std::max(1U, std::thread::hardware_concurrency());
}

// Repeatedly executes |function| multiple times, specified by |num_iterations|.
// Different executions of |function| may occur on one of the |num_threads|
// threads.
//
// |function| has a function signature of void(const size_t i), with |i| taking
// values in the range [0, |num_iterations|).
//
// @param num_threads Number of threads to execute |function|.
// @param num_iterations Number of iterations for this for loop.
// @param function Function to be run. The function should take a single
//     parameter that is of type const size_t, representing the iteration
//     index, and no return value.
void ParallelFor(unsigned int num_threads, size_t num_iterations,
                 const std::function<void(const size_t)>& function);

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PARALLEL_FOR_H_
