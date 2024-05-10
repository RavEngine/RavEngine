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

#include <cmath>

#include "base/logging.h"
#include "base/misc_math.h"
#include "geometrical_acoustics/estimating_rt60.h"

namespace vraudio {

// The RT60 is estimated by fitting a line to the log of energy. We collect
// energy from individual impulse responses into bins and use each bin's
// midpoint and height (average energy inside the bin) as a data point (x, y)
// for the line fitting algorithm. For example, for a bin that collects 100.0
// units of energy from 0.0s to 2.0s has a midpoint of 1.0s and height of
// log10(100.0 / 2.0).
//
// In general we have some liberty in how we divide the whole reverb tail as
// bins. For example, for a reverb tail of 3.5 seconds long:
//   ----------------------------------------------------------------------->
//
// we may divide it into 7 bins of widths 0.5 seconds:
//   |---------|---------|---------|---------|---------|---------|---------|>
//   0.0s      0.5s      1.0s      1.5s      2.0s      2.5s      3.0s      3.5s
//
// or we may divide it into 3 bins of widths {0.5s, 1.0s, 2.0s}:
//   |---------|-------------------|---------------------------------------|>
//   0.0s      0.5s                1.5s                                    3.5s
//
// In this implementation, we adopted a scheme that lets the accumulated energy
// determine the width. The first bin would contain 0.5 of the total energy of
// the whole reverb tail, the second bin would contain 0.5 of the remaining
// energy after the first bin, and so on. The number 0.5 can be replaced by any
// value in [0.0, 1.0), which is defined as |kBinEnergyRatio| in our
// implementation.
//
// The advantage of this scheme is that there will not be any empty bin (of
// height zero), which might occur because the impulse responses can be sparse
// at certain regions. An empty bin is a particularly bad data point and
// will greatly reduce the efficacy of the linear fitting. (For a more
// detailed discussion of the sparsity of impulse responses, see [internal ref]
float EstimateRT60(const std::vector<float>& energy_impulse_responses,
                   const float sampling_rate) {
  // First initialize the remaining energy to be the total energy.
  float remaining_energy = 0.0f;
  for (const float energy : energy_impulse_responses) {
    remaining_energy += energy;
  }

  // The collected data points: the bin's midpoints and heights.
  std::vector<float> bin_midpoints;
  std::vector<float> bin_heights;

  // The target energy (as a ratio of the remaining energy) that a bin should
  // collect before closing.
  const float kBinEnergyRatio = 0.5f;
  float target_accumulated_energy = remaining_energy * kBinEnergyRatio;

  // Stop when the remaining energy is below this threshold.
  const float kRemainingThreshold = remaining_energy * 1e-3f;

  // The iteration goes as follows:
  // 1. A bin is opened.
  // 2. Energy is accumulated from impulse responses.
  // 3. When the accumulated energy exceeds the target, the bin is closed and
  //    its midpoint and height recorded.
  // 4. Repeat 1-3 until the remaining energy is below a threshold.
  float accumulated_energy = 0.0f;
  float bin_begin = -1.0f;
  for (size_t i = 0; i < energy_impulse_responses.size(); ++i) {
    if (energy_impulse_responses[i] <= 0.0f) {
      continue;
    }

    // The first non-zero response is found; set the |bin_begin|.
    if (bin_begin < 0.0f) {
      bin_begin = static_cast<float>(i) / sampling_rate;
    }

    accumulated_energy += energy_impulse_responses[i];

    // Close the bin if the accumulated energy meets the target.
    if (accumulated_energy > target_accumulated_energy) {
      // Compute the bin's midpoint, in the unit of second.
      const float bin_end = static_cast<float>(i + 1) / sampling_rate;
      const float bin_midpoint = 0.5f * (bin_begin + bin_end);

      // Compute the bin's height as the average energy inside the bin, in the
      // unit of dB.
      const float bin_width = bin_end - bin_begin;
      const float bin_height =
          10.0f * std::log10(accumulated_energy / bin_width);

      // Collect the data point.
      bin_midpoints.push_back(bin_midpoint);
      bin_heights.push_back(bin_height);

      // Terminate the data point collection when the remaining energy is below
      // threshold.
      remaining_energy -= accumulated_energy;
      if (remaining_energy < kRemainingThreshold) {
        break;
      }

      // Start a new bin and update the remaining energy.
      target_accumulated_energy = remaining_energy * kBinEnergyRatio;
      bin_begin = bin_end;
      accumulated_energy = 0.0f;
    }
  }

  // Require at least some data points to perform linear fitting.
  const size_t kMinNumDataPointsForFitting = 3;
  if (bin_midpoints.size() < kMinNumDataPointsForFitting) {
    LOG(WARNING) << "Insufficient number of data points for fitting";
    return 0.0f;
  }

  // Perform linear fitting.
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_square = 0.0f;
  if (!LinearLeastSquareFitting(bin_midpoints, bin_heights, &slope, &intercept,
                                &r_square)) {
    LOG(WARNING) << "Linear least square fitting failed";
    return 0.0f;
  }
  LOG(INFO) << "Fitted slope= " << slope << "; intercept= " << intercept
            << "; R^2= " << r_square;

  // RT60 is defined as how long it takes for the energy to decay 60 dB.
  // Note that |slope| should be negative.
  if (slope < 0.0f) {
    return -60.0f / slope;
  } else {
    LOG(WARNING) << "Invalid RT60 from non-negative slope";
    return 0.0f;
  }
}

}  // namespace vraudio
